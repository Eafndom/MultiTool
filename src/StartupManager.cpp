#include "StartupManager.h"
#include "Logger.h"
#include <windows.h>
#include <string>

bool StartupManager::SetRunAtStartup(bool enable) {
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey);
    if (result != ERROR_SUCCESS) return false;

    if (enable) {
        WCHAR path[MAX_PATH];
        GetModuleFileNameW(NULL, path, MAX_PATH);
        std::wstring quotedPath = L"\"";
        quotedPath += path;
        quotedPath += L"\"";
        result = RegSetValueExW(hKey, L"GamingPerformanceControlCenter", 0, REG_SZ, (BYTE*)quotedPath.c_str(), (quotedPath.length() + 1) * sizeof(WCHAR));
    } else {
        result = RegDeleteValueW(hKey, L"GamingPerformanceControlCenter");
    }

    RegCloseKey(hKey);
    return result == ERROR_SUCCESS || result == ERROR_FILE_NOT_FOUND;
}

bool StartupManager::IsRunAtStartupEnabled() {
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS) return false;

    WCHAR path[MAX_PATH];
    DWORD pathSize = sizeof(path);
    result = RegQueryValueExW(hKey, L"GamingPerformanceControlCenter", NULL, NULL, (BYTE*)path, &pathSize);

    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}
