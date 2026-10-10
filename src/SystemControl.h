#pragma once
#include <windows.h>
#include <string>
#include <optional>
class SystemControl {
public:
    static SystemControl& Get() { static SystemControl instance; return instance; }
    bool SetProcessPriority(DWORD pid, DWORD priorityClass);
    bool SetProcessAffinity(DWORD pid, DWORD_PTR affinityMask);
    DWORD GetProcessPriority(DWORD pid);
    DWORD_PTR GetProcessAffinity(DWORD pid);
    bool SetActivePowerPlan(const GUID& planGuid);
    std::optional<GUID> GetActivePowerPlan();
    static bool ParseGuid(const std::string& str, GUID& guid);
    bool ChangeDisplaySettings(int width, int height, int refreshRate);
private:
    SystemControl() = default;
};
