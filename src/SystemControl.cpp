#include "SystemControl.h"
#include "Logger.h"
#include <powrprof.h>
#include <iostream>
#pragma comment(lib, "PowrProf.lib")
bool SystemControl::SetProcessPriority(DWORD pid, DWORD priorityClass) {
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
    if (!hProcess) return false;
    bool res = SetPriorityClass(hProcess, priorityClass);
    CloseHandle(hProcess);
    return res;
}
bool SystemControl::SetProcessAffinity(DWORD pid, DWORD_PTR affinityMask) {
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
    if (!hProcess) return false;
    bool res = SetProcessAffinityMask(hProcess, affinityMask);
    CloseHandle(hProcess);
    return res;
}
DWORD SystemControl::GetProcessPriority(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (hProcess) {
        DWORD prio = GetPriorityClass(hProcess);
        CloseHandle(hProcess);
        return prio;
    }
    return NORMAL_PRIORITY_CLASS;
}
DWORD_PTR SystemControl::GetProcessAffinity(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (hProcess) {
        DWORD_PTR procMask = 0, sysMask = 0;
        if (GetProcessAffinityMask(hProcess, &procMask, &sysMask)) {
            CloseHandle(hProcess);
            return procMask;
        }
        CloseHandle(hProcess);
    }
    return 0;
}
bool SystemControl::SetActivePowerPlan(const GUID& planGuid) {
    return PowerSetActiveScheme(NULL, &planGuid) == ERROR_SUCCESS;
}
std::optional<GUID> SystemControl::GetActivePowerPlan() {
    GUID* activePolicyGuid = nullptr;
    if (PowerGetActiveScheme(NULL, &activePolicyGuid) == ERROR_SUCCESS && activePolicyGuid) {
        GUID ret = *activePolicyGuid;
        LocalFree(activePolicyGuid);
        return ret;
    }
    return std::nullopt;
}
bool SystemControl::ParseGuid(const std::string& str, GUID& guid) {
    if (str.length() != 36) return false;
    auto parseHex = [](const char* p, int len) -> unsigned long {
        unsigned long val = 0;
        for (int i=0; i<len; ++i) {
            char c = p[i];
            val <<= 4;
            if (c >= '0' && c <= '9') val |= (c - '0');
            else if (c >= 'a' && c <= 'f') val |= (c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') val |= (c - 'A' + 10);
            else return 0;
        }
        return val;
    };
    guid.Data1 = parseHex(str.c_str(), 8);
    guid.Data2 = parseHex(str.c_str() + 9, 4);
    guid.Data3 = parseHex(str.c_str() + 14, 4);
    unsigned long d4_1 = parseHex(str.c_str() + 19, 4);
    guid.Data4[0] = (d4_1 >> 8) & 0xFF; guid.Data4[1] = d4_1 & 0xFF;
    unsigned long d4_2 = parseHex(str.c_str() + 24, 8);
    unsigned long d4_3 = parseHex(str.c_str() + 32, 4);
    guid.Data4[2] = (d4_2 >> 24) & 0xFF; guid.Data4[3] = (d4_2 >> 16) & 0xFF;
    guid.Data4[4] = (d4_2 >> 8) & 0xFF; guid.Data4[5] = d4_2 & 0xFF;
    guid.Data4[6] = (d4_3 >> 8) & 0xFF; guid.Data4[7] = d4_3 & 0xFF;
    return true;
}
bool SystemControl::ChangeDisplaySettings(int width, int height, int refreshRate) {
    DEVMODEW dm; ZeroMemory(&dm, sizeof(dm)); dm.dmSize = sizeof(dm);
    bool found = false; int modeNum = 0;
    while (EnumDisplaySettingsW(NULL, modeNum, &dm)) {
        if ((int)dm.dmPelsWidth == width && (int)dm.dmPelsHeight == height && (int)dm.dmDisplayFrequency == refreshRate) {
            found = true; break;
        }
        modeNum++;
    }
    if (!found) return false;
    return ChangeDisplaySettingsExW(NULL, &dm, NULL, CDS_FULLSCREEN, NULL) == DISP_CHANGE_SUCCESSFUL;
}
