#include "PerformanceMonitor.h"
#include <psapi.h>
bool PerformanceMonitor::Initialize() {
    if (PdhOpenQuery(NULL, 0, &m_query) != ERROR_SUCCESS) return false;
    if (PdhAddEnglishCounterW(m_query, L"\\Processor(_Total)\\% Processor Time", 0, &m_cpuCounter) != ERROR_SUCCESS) return false;
    // GPU counter might fail if hardware doesn't support it or translation fails, so we don't abort on error here
    PdhAddEnglishCounterW(m_query, L"\\GPU Engine(*)\\Utilization Percentage", 0, &m_gpuCounter);
    PdhCollectQueryData(m_query);
    return true;
}
PerformanceMonitor::~PerformanceMonitor() {
    if (m_query) PdhCloseQuery(m_query);
}
void PerformanceMonitor::SetGameProcess(DWORD pid, const std::wstring& processName) {
    m_gamePid = pid; m_gameProcessName = processName;
}
void PerformanceMonitor::Update() {
    if (!m_query) return;
    PdhCollectQueryData(m_query);
    PDH_FMT_COUNTERVALUE counterVal;
    if (PdhGetFormattedCounterValue(m_cpuCounter, PDH_FMT_DOUBLE, NULL, &counterVal) == ERROR_SUCCESS) {
        m_currentMetrics.cpuUtilization = counterVal.doubleValue;
    }

    if (m_gpuCounter != NULL) {
        if (PdhGetFormattedCounterValue(m_gpuCounter, PDH_FMT_DOUBLE, NULL, &counterVal) == ERROR_SUCCESS) {
            m_currentMetrics.gpuUtilization = counterVal.doubleValue;
        }
    }
    MEMORYSTATUSEX memInfo; memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        m_currentMetrics.ramUsageMB = (memInfo.ullTotalPhys - memInfo.ullAvailPhys) / (1024.0 * 1024.0);
        m_currentMetrics.availableRamMB = memInfo.ullAvailPhys / (1024.0 * 1024.0);
    }
    if (m_gamePid != 0) {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, m_gamePid);
        if (hProcess) {
            PROCESS_MEMORY_COUNTERS pmc;
            if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                m_currentMetrics.gameProcessRamUsageMB = pmc.WorkingSetSize / (1024.0 * 1024.0);
            }
            CloseHandle(hProcess);
        }
    }
}
SystemMetrics PerformanceMonitor::GetMetrics() const { return m_currentMetrics; }
