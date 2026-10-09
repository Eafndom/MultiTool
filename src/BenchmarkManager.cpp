#include "BenchmarkManager.h"
#include "Logger.h"

void BenchmarkManager::StartBenchmark(const std::string& gameName, const std::string& profileName) {
    if (m_isRunning) return;
    m_isRunning = true;
    m_currentProfile = profileName;
    SessionRecorder::Get().StartSession(gameName);
    Logger::Get().Info("Started benchmark run for profile: " + profileName);
}

void BenchmarkManager::EndBenchmark() {
    if (!m_isRunning) return;
    m_isRunning = false;
    SessionRecorder::Get().EndSession();

    BenchmarkRun run;
    run.profileName = m_currentProfile;
    run.stats = SessionRecorder::Get().GetLastSessionStats();
    m_runs.push_back(run);

    Logger::Get().Info("Ended benchmark run for profile: " + m_currentProfile);
}
