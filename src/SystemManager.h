#pragma once
#include <windows.h>
#include <functional>

class SystemManager {
public:
    static SystemManager& Get() {
        static SystemManager instance;
        return instance;
    }

    bool Initialize(HWND hwnd);
    void Shutdown();

    void RegisterHotkeys();
    void UnregisterHotkeys();
    void HandleHotkey(int hotkeyId);

    void ShowTrayIcon();
    void HideTrayIcon();

    std::function<void()> OnToggleLimiter;
    std::function<void()> OnIncreaseFPS;
    std::function<void()> OnDecreaseFPS;
    std::function<void()> OnShowUI;
    std::function<void()> OnExit;

private:
    SystemManager() = default;
    ~SystemManager() = default;

    HWND m_hwnd = nullptr;
    NOTIFYICONDATAW m_nid = {};
    bool m_trayIconVisible = false;

    enum HotkeyIDs {
        HK_TOGGLE = 1,
        HK_INC_FPS = 2,
        HK_DEC_FPS = 3,
        HK_SHOW_UI = 4
    };
};
