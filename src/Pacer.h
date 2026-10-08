#pragma once
#include <windows.h>
#include <vector>
#include <mutex>
#include <cstdint>
#include <chrono>

struct FrameStats {
    double currentFPS;
    double averageFPS;
    double currentFrameTimeMs;
    double averageFrameTimeMs;
    double targetFrameTimeMs;
    double onePercentLowMs;
    double zeroOnePercentLowMs;
    double minFrameTimeMs;
    double maxFrameTimeMs;
    int stutterCount;
};

class Pacer {
public:
    Pacer();
    ~Pacer();

    void Initialize();
    void SetTargetFPS(double fps);
    void MarkFrameStart();
    void WaitAndPace();

    FrameStats GetStats() const;
    const std::vector<double>& GetFrameTimeHistory() const;

    bool IsEnabled() const { return m_enabled; }
    void SetEnabled(bool enabled) { m_enabled = enabled; }

    static Pacer& Get() {
        static Pacer instance;
        return instance;
    }

private:
    void CalculateStats();
    double GetCurrentTimeMs() const;
    long long GetCurrentQPC() const;

    LARGE_INTEGER m_qpcFrequency;
    long long m_targetIntervalTicks;
    long long m_nextFrameQPC;

    HANDLE m_waitableTimer;
    bool m_enabled;
    double m_targetFPS;

    mutable std::mutex m_statsMutex;
    std::vector<double> m_frameTimeHistory;
    size_t m_historyHead;
    static const size_t HISTORY_SIZE = 1000;

    mutable FrameStats m_currentStats;
    int m_stutterCount;

    long long m_lastFrameEndQPC;
    bool m_firstFrame;
};
