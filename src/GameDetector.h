#pragma once
#include <windows.h>
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>

class GameDetector {
public:
    static GameDetector& Get() {
        static GameDetector instance;
        return instance;
    }

    void Start();
    void Stop();

    std::wstring GetCurrentGameName();
    DWORD GetCurrentGamePID();

    std::function<void(const std::wstring&, DWORD)> OnGameDetected;
    std::function<void()> OnGameLost;

private:
    GameDetector() : m_running(false), m_currentGamePID(0) {}
    ~GameDetector() { Stop(); }

    void DetectionLoop();
    bool IsGameWindow(HWND hwnd);

    std::atomic<bool> m_running;
    std::thread m_detectionThread;

    std::mutex m_mutex;
    std::wstring m_currentGameName;
    DWORD m_currentGamePID;
};
