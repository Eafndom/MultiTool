#include "GameDetector.h"
#include <psapi.h>
#include <iostream>

void GameDetector::Start() {
    if (m_running) return;
    m_running = true;
    m_detectionThread = std::thread(&GameDetector::DetectionLoop, this);
}

void GameDetector::Stop() {
    if (!m_running) return;
    m_running = false;
    if (m_detectionThread.joinable()) {
        m_detectionThread.join();
    }
}

std::wstring GameDetector::GetCurrentGameName() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentGameName;
}

DWORD GameDetector::GetCurrentGamePID() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentGamePID;
}

bool GameDetector::IsGameWindow(HWND hwnd) {
    if (!IsWindowVisible(hwnd)) return false;

    RECT rect;
    GetWindowRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    if (width < 800 || height < 600) return false;

    DWORD style = GetWindowLong(hwnd, GWL_STYLE);
    if ((style & WS_POPUP) != 0 || (style & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW) {
        return true;
    }

    return false;
}

void GameDetector::DetectionLoop() {
    while (m_running) {
        HWND foregroundWindow = GetForegroundWindow();
        if (foregroundWindow && foregroundWindow != GetDesktopWindow() && foregroundWindow != GetShellWindow()) {
            if (IsGameWindow(foregroundWindow)) {
                DWORD pid;
                GetWindowThreadProcessId(foregroundWindow, &pid);

                bool newGame = false;
                std::wstring processName;

                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    if (pid != m_currentGamePID) {
                        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
                        if (hProcess) {
                            WCHAR buffer[MAX_PATH];
                            if (GetModuleFileNameExW(hProcess, NULL, buffer, MAX_PATH)) {
                                processName = buffer;
                                size_t pos = processName.find_last_of(L"\\/");
                                if (pos != std::wstring::npos) {
                                    processName = processName.substr(pos + 1);
                                }
                                m_currentGameName = processName;
                                m_currentGamePID = pid;
                                newGame = true;
                            }
                            CloseHandle(hProcess);
                        }
                    }
                }

                if (newGame && OnGameDetected) {
                    OnGameDetected(processName, pid);
                }
            } else {
                bool lost = false;
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    if (m_currentGamePID != 0) {
                        m_currentGamePID = 0;
                        m_currentGameName = L"";
                        lost = true;
                    }
                }
                if (lost && OnGameLost) {
                    OnGameLost();
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}
