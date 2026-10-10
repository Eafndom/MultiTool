#pragma once
#include <windows.h>
#include <pdh.h>
#include <string>
struct SystemMetrics {
    double cpuUtilization = 0;
    double ramUsageMB = 0;
    double availableRamMB = 0;
    double gameProcessCpuUsage = 0;
    double gameProcessRamUsageMB = 0;
    double gpuUtilization = 0;
};
class PerformanceMonitor {
public:
    static PerformanceMonitor& Get() { static PerformanceMonitor instance; return instance; }
    bool Initialize();
    void SetGameProcess(DWORD pid, const std::wstring& processName);
    void Update();
    SystemMetrics GetMetrics() const;
private:
    PerformanceMonitor() : m_query(NULL), m_cpuCounter(NULL) {}
    ~PerformanceMonitor();
    PDH_HQUERY m_query;
    PDH_HCOUNTER m_cpuCounter;
    PDH_HCOUNTER m_gpuCounter;
    DWORD m_gamePid = 0;
    std::wstring m_gameProcessName;
    SystemMetrics m_currentMetrics;
};
