#include "SystemManager.h"

#define WM_TRAYICON (WM_USER + 1)

bool SystemManager::Initialize(HWND hwnd) {
    m_hwnd = hwnd;
    return true;
}

void SystemManager::Shutdown() {
    HideTrayIcon();
    UnregisterHotkeys();
}

void SystemManager::ShowTrayIcon() {
    if (m_trayIconVisible || !m_hwnd) return;

    m_nid.cbSize = sizeof(NOTIFYICONDATAW);
    m_nid.hWnd = m_hwnd;
    m_nid.uID = 1;
    m_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAYICON;
    m_nid.hIcon = LoadIcon(NULL, IDI_APPLICATION); // Default icon for now
    lstrcpyW(m_nid.szTip, L"FramePacerClone");

    Shell_NotifyIconW(NIM_ADD, &m_nid);
    m_trayIconVisible = true;
}

void SystemManager::HideTrayIcon() {
    if (!m_trayIconVisible) return;
    Shell_NotifyIconW(NIM_DELETE, &m_nid);
    m_trayIconVisible = false;
}

void SystemManager::RegisterHotkeys() {
    if (!m_hwnd) return;

    RegisterHotKey(m_hwnd, HK_TOGGLE, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, 'L');
    RegisterHotKey(m_hwnd, HK_INC_FPS, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, VK_UP);
    RegisterHotKey(m_hwnd, HK_DEC_FPS, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, VK_DOWN);
    RegisterHotKey(m_hwnd, HK_SHOW_UI, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, 'O');
}

void SystemManager::UnregisterHotkeys() {
    if (!m_hwnd) return;
    UnregisterHotKey(m_hwnd, HK_TOGGLE);
    UnregisterHotKey(m_hwnd, HK_INC_FPS);
    UnregisterHotKey(m_hwnd, HK_DEC_FPS);
    UnregisterHotKey(m_hwnd, HK_SHOW_UI);
}

void SystemManager::HandleHotkey(int hotkeyId) {
    switch (hotkeyId) {
        case HK_TOGGLE: if (OnToggleLimiter) OnToggleLimiter(); break;
        case HK_INC_FPS: if (OnIncreaseFPS) OnIncreaseFPS(); break;
        case HK_DEC_FPS: if (OnDecreaseFPS) OnDecreaseFPS(); break;
        case HK_SHOW_UI: if (OnShowUI) OnShowUI(); break;
    }
}
