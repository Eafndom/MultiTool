#include "SessionRecorder.h"
#include <algorithm>
#include <numeric>
void SessionRecorder::StartSession(const std::string& gameName) {
    if (m_isRecording) return;
    m_isRecording = true; m_currentGame = gameName; m_frameTimes.clear();
    m_startTime = std::chrono::steady_clock::now();
}
void SessionRecorder::RecordFrame(double frameTimeMs) {
    if (!m_isRecording) return;
    m_frameTimes.push_back(frameTimeMs);
}
void SessionRecorder::EndSession() {
    if (!m_isRecording) return;
    m_isRecording = false;
    auto endTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> duration = endTime - m_startTime;
    m_lastStats.gameName = m_currentGame; m_lastStats.durationSeconds = duration.count();
    if (m_frameTimes.empty()) return;
    double totalTime = std::accumulate(m_frameTimes.begin(), m_frameTimes.end(), 0.0);
    m_lastStats.averageFps = 1000.0 / (totalTime / m_frameTimes.size());
    std::vector<double> sortedFrames = m_frameTimes;
    std::sort(sortedFrames.begin(), sortedFrames.end(), std::greater<double>());
    size_t onePercentIndex = std::max((size_t)0, (size_t)(sortedFrames.size() * 0.01));
    size_t zeroOnePercentIndex = std::max((size_t)0, (size_t)(sortedFrames.size() * 0.001));
    m_lastStats.onePercentLowFps = 1000.0 / sortedFrames[onePercentIndex];
    m_lastStats.zeroOnePercentLowFps = 1000.0 / sortedFrames[zeroOnePercentIndex];
    m_lastStats.stutterEvents = 0;
    double avgFrameTime = totalTime / m_frameTimes.size();
    for (double ft : m_frameTimes) if (ft > avgFrameTime * 2.0) m_lastStats.stutterEvents++;
}
