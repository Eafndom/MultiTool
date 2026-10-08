#include "Pacer.h"
#include <algorithm>
#include <numeric>

#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif

Pacer::Pacer() : m_enabled(true), m_targetFPS(120.0), m_historyHead(0), m_stutterCount(0), m_firstFrame(true) {
    QueryPerformanceFrequency(&m_qpcFrequency);
    m_frameTimeHistory.resize(HISTORY_SIZE, 0.0);

    m_waitableTimer = CreateWaitableTimerExW(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
    if (!m_waitableTimer) {
        m_waitableTimer = CreateWaitableTimerW(NULL, TRUE, NULL);
    }

    SetTargetFPS(m_targetFPS);
}

Pacer::~Pacer() {
    if (m_waitableTimer) {
        CloseHandle(m_waitableTimer);
    }
}

void Pacer::Initialize() {
    m_firstFrame = true;
    m_stutterCount = 0;
}

void Pacer::SetTargetFPS(double fps) {
    std::lock_guard<std::mutex> lock(m_statsMutex);
    m_targetFPS = fps;
    if (fps > 0.0) {
        m_targetIntervalTicks = static_cast<long long>(m_qpcFrequency.QuadPart / fps);
    } else {
        m_targetIntervalTicks = 0;
    }
    m_firstFrame = true;
}

long long Pacer::GetCurrentQPC() const {
    LARGE_INTEGER qpc;
    QueryPerformanceCounter(&qpc);
    return qpc.QuadPart;
}

void Pacer::MarkFrameStart() {
}

void Pacer::WaitAndPace() {
    if (!m_enabled || m_targetIntervalTicks == 0) {
        long long currentQPC = GetCurrentQPC();
        if (!m_firstFrame) {
            double frameTimeMs = (currentQPC - m_lastFrameEndQPC) * 1000.0 / m_qpcFrequency.QuadPart;
            std::lock_guard<std::mutex> lock(m_statsMutex);
            m_frameTimeHistory[m_historyHead] = frameTimeMs;
            m_historyHead = (m_historyHead + 1) % HISTORY_SIZE;
        }
        m_lastFrameEndQPC = currentQPC;
        m_firstFrame = false;
        return;
    }

    long long currentQPC = GetCurrentQPC();

    if (m_firstFrame) {
        m_nextFrameQPC = currentQPC + m_targetIntervalTicks;
        m_lastFrameEndQPC = currentQPC;
        m_firstFrame = false;
        return;
    }

    long long diffQPC = m_nextFrameQPC - currentQPC;

    if (diffQPC > 0) {
        double diffMs = static_cast<double>(diffQPC) * 1000.0 / m_qpcFrequency.QuadPart;

        if (diffMs > 1.5 && m_waitableTimer) {
            double sleepMs = diffMs - 1.0;
            LARGE_INTEGER dueTime;
            dueTime.QuadPart = static_cast<LONGLONG>(-sleepMs * 10000.0);

            if (SetWaitableTimerEx(m_waitableTimer, &dueTime, 0, NULL, NULL, NULL, 0)) {
                WaitForSingleObject(m_waitableTimer, INFINITE);
            }
        }

        while (GetCurrentQPC() < m_nextFrameQPC) {
            YieldProcessor();
        }
    } else if (diffQPC < -m_targetIntervalTicks) {
        m_nextFrameQPC = currentQPC;
    }

    currentQPC = GetCurrentQPC();
    double frameTimeMs = (currentQPC - m_lastFrameEndQPC) * 1000.0 / m_qpcFrequency.QuadPart;

    m_lastFrameEndQPC = currentQPC;
    m_nextFrameQPC += m_targetIntervalTicks;

    {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_frameTimeHistory[m_historyHead] = frameTimeMs;
        m_historyHead = (m_historyHead + 1) % HISTORY_SIZE;

        double targetMs = 1000.0 / m_targetFPS;
        if (frameTimeMs > targetMs * 1.2) {
            m_stutterCount++;
        }
    }
}

FrameStats Pacer::GetStats() const {
    std::lock_guard<std::mutex> lock(m_statsMutex);

    FrameStats stats = {};
    stats.targetFrameTimeMs = m_targetFPS > 0.0 ? 1000.0 / m_targetFPS : 0.0;
    stats.stutterCount = m_stutterCount;

    std::vector<double> validTimes;
    validTimes.reserve(HISTORY_SIZE);

    double sum = 0.0;
    for (size_t i = 0; i < HISTORY_SIZE; ++i) {
        if (m_frameTimeHistory[i] > 0.0) {
            validTimes.push_back(m_frameTimeHistory[i]);
            sum += m_frameTimeHistory[i];
        }
    }

    if (validTimes.empty()) {
        return stats;
    }

    stats.currentFrameTimeMs = m_frameTimeHistory[(m_historyHead == 0 ? HISTORY_SIZE : m_historyHead) - 1];
    stats.currentFPS = stats.currentFrameTimeMs > 0.0 ? 1000.0 / stats.currentFrameTimeMs : 0.0;

    stats.averageFrameTimeMs = sum / validTimes.size();
    stats.averageFPS = stats.averageFrameTimeMs > 0.0 ? 1000.0 / stats.averageFrameTimeMs : 0.0;

    std::sort(validTimes.begin(), validTimes.end(), std::greater<double>());

    stats.maxFrameTimeMs = validTimes.front();
    stats.minFrameTimeMs = validTimes.back();

    size_t onePercentIndex = validTimes.size() / 100;
    size_t zeroOnePercentIndex = validTimes.size() / 1000;

    stats.onePercentLowMs = validTimes[onePercentIndex];
    stats.zeroOnePercentLowMs = validTimes[zeroOnePercentIndex];

    m_currentStats = stats;
    return stats;
}

const std::vector<double>& Pacer::GetFrameTimeHistory() const {
    return m_frameTimeHistory;
}
