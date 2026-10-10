#include <windows.h>
#include <iostream>
#include <thread>
#include <string>
#include <d3d11.h>
#include <tchar.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "Pacer.h"
#include "SystemManager.h"
#include "GameDetector.h"
#include "UI.h"
#include "DataManager.h"
#include "PerformanceMonitor.h"
#include "RuleEngine.h"
#include "Logger.h"
#include "SessionRecorder.h"
#include "StateRestorer.h"
#include "SystemControl.h"
#include <locale>
#include <codecvt>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
    return true;
}

void CleanupDeviceD3D()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    if (msg == WM_HOTKEY) {
        SystemManager::Get().HandleHotkey((int)wParam);
        return 0;
    }

    if (msg == (WM_USER + 1)) {
        if (lParam == WM_LBUTTONDBLCLK || lParam == WM_RBUTTONUP) {
            ShowWindow(hWnd, SW_RESTORE);
            SetForegroundWindow(hWnd);
        }
        return 0;
    }

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam);
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        if ((wParam & 0xfff0) == SC_MINIMIZE) {
            ShowWindow(hWnd, SW_HIDE);
            return 0;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

void RunSyntheticBenchmark(double targetFPS);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    std::string cmd(lpCmdLine);
    if (cmd == "--benchmark") {
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        RunSyntheticBenchmark(60.0);
        RunSyntheticBenchmark(120.0);
        RunSyntheticBenchmark(144.0);
        return 0;
    }

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"FramePacerClone", nullptr };
    RegisterClassExW(&wc);
    HWND hwnd = CreateWindowW(wc.lpszClassName, L"FramePacerClone", WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    SystemManager& sys = SystemManager::Get();
    sys.Initialize(hwnd);
    sys.ShowTrayIcon();
    sys.RegisterHotkeys();

    Pacer& pacer = Pacer::Get();
    pacer.Initialize();

    Logger::Get().Init("log.txt");
    DataManager::Get().Load();
    PerformanceMonitor::Get().Initialize();

    GameDetector& detector = GameDetector::Get();
    detector.OnGameDetected = [&](const std::wstring& processName, DWORD pid) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::string gameName = converter.to_bytes(processName);
        PerformanceMonitor::Get().SetGameProcess(pid, processName);
        StateRestorer::Get().CaptureState(pid);

        auto gameOpt = DataManager::Get().GetGameByProcessName(gameName);
        if (gameOpt && gameOpt->autoApply) {
            auto profileOpt = DataManager::Get().GetProfileByName(gameOpt->profileName);
            if (profileOpt) {
                const auto& p = profileOpt.value();
                DWORD origPrio = SystemControl::Get().GetProcessPriority(pid);
                DWORD_PTR origAffinity = SystemControl::Get().GetProcessAffinity(pid);
                StateRestorer::Get().RegisterModifiedProcess(pid, origPrio, origAffinity);

                SystemControl::Get().SetProcessPriority(pid, p.processPriority);
                if (p.processAffinity != 0) SystemControl::Get().SetProcessAffinity(pid, p.processAffinity);
                if (p.displayWidth > 0 && p.displayHeight > 0 && p.displayRefreshRate > 0) {
                    SystemControl::Get().ChangeDisplaySettings(p.displayWidth, p.displayHeight, p.displayRefreshRate);
                }
                if (!p.powerPlanGuid.empty()) {
                    GUID guid;
                    if (SystemControl::ParseGuid(p.powerPlanGuid, guid)) SystemControl::Get().SetActivePowerPlan(guid);
                }
                if (p.targetFPS > 0.0) { Pacer::Get().SetTargetFPS(p.targetFPS); Pacer::Get().SetEnabled(true); }
            }
        }
    };

    detector.OnGameLost = [&]() {
        PerformanceMonitor::Get().SetGameProcess(0, L"");
        StateRestorer::Get().RestoreState();
    };

    detector.Start();

    double currentTargetFPS = 120.0;

    sys.OnToggleLimiter = [&]() { pacer.SetEnabled(!pacer.IsEnabled()); };
    sys.OnIncreaseFPS = [&]() { currentTargetFPS += 5.0; pacer.SetTargetFPS(currentTargetFPS); };
    sys.OnDecreaseFPS = [&]() { currentTargetFPS = std::max(30.0, currentTargetFPS - 5.0); pacer.SetTargetFPS(currentTargetFPS); };
    sys.OnShowUI = [&]() { ShowWindow(hwnd, SW_RESTORE); SetForegroundWindow(hwnd); };

    bool done = false;
    while (!done)
    {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done) break;

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            ID3D11Texture2D* pBackBuffer;
            g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
            g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
            pBackBuffer->Release();
            g_ResizeWidth = g_ResizeHeight = 0;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("GamingPerformanceControlCenter", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        if (ImGui::BeginTabBar("MainTabs")) {
            if (ImGui::BeginTabItem("Dashboard")) { UI::RenderDashboard(); ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Games")) {
                ImGui::Text("Games & Triggers");
                ImGui::Separator();
                auto& games = DataManager::Get().games;
                for (auto& g : games) {
                    if (ImGui::TreeNode(g.processName.c_str())) {
                        char nameBuf[256]; strncpy(nameBuf, g.processName.c_str(), sizeof(nameBuf)); nameBuf[sizeof(nameBuf)-1] = 0;
                        if (ImGui::InputText("Process Name", nameBuf, sizeof(nameBuf))) g.processName = nameBuf;
                        char profBuf[256]; strncpy(profBuf, g.profileName.c_str(), sizeof(profBuf)); profBuf[sizeof(profBuf)-1] = 0;
                        if (ImGui::InputText("Profile To Apply", profBuf, sizeof(profBuf))) g.profileName = profBuf;
                        ImGui::Checkbox("Auto Apply", &g.autoApply);
                        ImGui::TreePop();
                    }
                }
                if (ImGui::Button("Add Game Trigger")) {
                    GameSettings g; g.processName = "NewGame.exe"; g.profileName = "Default"; games.push_back(g);
                }
                if (ImGui::Button("Save Data")) DataManager::Get().Save();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Profiles")) { UI::RenderProfileEditor(); ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Rules")) { UI::RenderRuleEditor(); ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Session Analysis")) { UI::RenderSessionAnalysis(); ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Benchmark")) { UI::RenderBenchmarkTab(); ImGui::EndTabItem(); }
            ImGui::EndTabBar();
        }

        ImGui::End();

        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(0, 0);

        pacer.WaitAndPace();

        if (SessionRecorder::Get().IsRecording()) {
            SessionRecorder::Get().RecordFrame(pacer.GetStats().currentFrameTimeMs);
        }

        static auto lastUpdate = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate).count() >= 1000) {
            PerformanceMonitor::Get().Update();
            std::wstring wGameName = detector.GetCurrentGameName();
            std::string gameName = "None";
            if (!wGameName.empty()) {
                std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                gameName = converter.to_bytes(wGameName);
            }
            RuleEngine::Get().Evaluate(PerformanceMonitor::Get().GetMetrics(), gameName);
            lastUpdate = now;
        }
    }

    detector.Stop();
    sys.Shutdown();

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

void RunSyntheticBenchmark(double targetFPS) {
    std::cout << "Starting benchmark for " << targetFPS << " FPS..." << std::endl;
    Pacer pacer;
    pacer.SetTargetFPS(targetFPS);
    pacer.Initialize();
    auto startTime = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - startTime < std::chrono::seconds(2)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        pacer.WaitAndPace();
    }
    FrameStats stats = pacer.GetStats();
    std::cout << "TARGET: " << targetFPS << " FPS\n";
    std::cout << "AVERAGE: " << stats.averageFrameTimeMs << " ms\n";
    std::cout << "---------------------------------\n";
}
