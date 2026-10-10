#pragma once
#include <string>
#include <vector>
#include <chrono>
struct SessionStats {
    std::string gameName;
    double durationSeconds;
    double averageFps;
    double onePercentLowFps;
    double zeroOnePercentLowFps;
    int stutterEvents;
};
class SessionRecorder {
public:
    static SessionRecorder& Get() { static SessionRecorder instance; return instance; }
    void StartSession(const std::string& gameName);
    void RecordFrame(double frameTimeMs);
    void EndSession();
    SessionStats GetLastSessionStats() const { return m_lastStats; }
    bool IsRecording() const { return m_isRecording; }
private:
    SessionRecorder() = default;
    bool m_isRecording = false;
    std::string m_currentGame;
    std::vector<double> m_frameTimes;
    std::chrono::steady_clock::time_point m_startTime;
    SessionStats m_lastStats = {};
};
