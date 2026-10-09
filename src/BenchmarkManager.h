#pragma once
#include <string>
#include <vector>
#include "SessionRecorder.h"

struct BenchmarkRun {
    std::string profileName;
    SessionStats stats;
};

class BenchmarkManager {
public:
    static BenchmarkManager& Get() { static BenchmarkManager instance; return instance; }

    void StartBenchmark(const std::string& gameName, const std::string& profileName);
    void EndBenchmark();

    const std::vector<BenchmarkRun>& GetRuns() const { return m_runs; }
    bool IsRunning() const { return m_isRunning; }
    void ClearRuns() { m_runs.clear(); }

private:
    BenchmarkManager() = default;

    bool m_isRunning = false;
    std::string m_currentProfile;
    std::vector<BenchmarkRun> m_runs;
};
