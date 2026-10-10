#pragma once
#include <windows.h>
#include <optional>
#include <vector>
struct ProcessState { DWORD pid; DWORD originalPriority; DWORD_PTR originalAffinity; };
class StateRestorer {
public:
    static StateRestorer& Get() { static StateRestorer instance; return instance; }
    void CaptureState(DWORD pid);
    void RestoreState();
    void RegisterModifiedProcess(DWORD pid, DWORD origPriority, DWORD_PTR origAffinity);
private:
    StateRestorer() = default;
    std::optional<GUID> m_originalPowerPlan;
    std::optional<DEVMODEW> m_originalDisplaySettings;
    std::vector<ProcessState> m_modifiedProcesses;
};
