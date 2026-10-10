#include "UI.h"
#include "imgui.h"
#include "PerformanceMonitor.h"
#include "SessionRecorder.h"
#include "DataManager.h"
#include "GameDetector.h"
#include "RuleEngine.h"
#include "BenchmarkManager.h"
#include "StartupManager.h"
#include <codecvt>
void UI::RenderDashboard() {
    std::wstring wGameName = GameDetector::Get().GetCurrentGameName();
    std::string gameName = "None";
    if (!wGameName.empty()) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        gameName = converter.to_bytes(wGameName);
    }
    ImGui::Text("GAMING PERFORMANCE CENTER");
    ImGui::Separator();
    ImGui::Text("Current Game: %s", gameName.c_str());
    auto metrics = PerformanceMonitor::Get().GetMetrics();
    ImGui::Text("CPU: %.1f%%", metrics.cpuUtilization);
    ImGui::Text("GPU: %.1f%%", metrics.gpuUtilization);
    ImGui::Text("RAM: %.1f GB / %.1f GB", metrics.ramUsageMB / 1024.0, (metrics.ramUsageMB + metrics.availableRamMB) / 1024.0);
    if (SessionRecorder::Get().IsRecording()) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Recording in progress...");
        if (ImGui::Button("Stop Recording")) SessionRecorder::Get().EndSession();
    } else {
        if (ImGui::Button("Start Recording")) SessionRecorder::Get().StartSession(gameName);
    }

    ImGui::Separator();
    bool startup = StartupManager::IsRunAtStartupEnabled();
    if (ImGui::Checkbox("Run at Windows Startup", &startup)) {
        StartupManager::SetRunAtStartup(startup);
    }
}
void UI::RenderProfileEditor() {
    ImGui::Text("Profile Editor");
    ImGui::Separator();
    auto& profiles = DataManager::Get().profiles;
    for (auto& p : profiles) {
        if (ImGui::TreeNode(p.name.c_str())) {
            char nameBuf[256]; strncpy(nameBuf, p.name.c_str(), sizeof(nameBuf)); nameBuf[sizeof(nameBuf)-1] = 0;
            if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) p.name = nameBuf;
            char planBuf[256]; strncpy(planBuf, p.powerPlanGuid.c_str(), sizeof(planBuf)); planBuf[sizeof(planBuf)-1] = 0;
            if (ImGui::InputText("Power Plan GUID", planBuf, sizeof(planBuf))) p.powerPlanGuid = planBuf;
            int prio = p.processPriority; if (ImGui::InputInt("Process Priority Class", &prio)) p.processPriority = prio;
            ImGui::InputInt("Display Width", &p.displayWidth);
            ImGui::InputInt("Display Height", &p.displayHeight);
            ImGui::InputInt("Refresh Rate", &p.displayRefreshRate);
            ImGui::InputDouble("Target FPS", &p.targetFPS);
            if (ImGui::Button("Save Profiles")) DataManager::Get().Save();
            ImGui::TreePop();
        }
    }
}
void UI::RenderRuleEditor() {
    ImGui::Text("Rule Editor");
    ImGui::Separator();
    auto& rules = RuleEngine::Get().rules;
    for (auto& r : rules) {
        if (ImGui::TreeNode(r.name.c_str())) {
            ImGui::Checkbox("Enabled", &r.enabled);
            const char* conditions[] = { "CPU > %", "RAM > MB", "Game Running" };
            int condIdx = (int)r.condition.type;
            if (ImGui::Combo("Condition", &condIdx, conditions, IM_ARRAYSIZE(conditions))) r.condition.type = (ConditionType)condIdx;
            if (r.condition.type == ConditionType::GAME_PROCESS_RUNNING) {
                char strBuf[256]; strncpy(strBuf, r.condition.stringValue.c_str(), sizeof(strBuf)); strBuf[sizeof(strBuf)-1] = 0;
                if (ImGui::InputText("Game Executable", strBuf, sizeof(strBuf))) r.condition.stringValue = strBuf;
            } else {
                ImGui::InputDouble("Threshold", &r.condition.thresholdValue);
            }
            const char* actions[] = { "Apply Profile", "Set Priority" };
            int actIdx = (int)r.action.type;
            if (ImGui::Combo("Action", &actIdx, actions, IM_ARRAYSIZE(actions))) r.action.type = (ActionType)actIdx;
            if (r.action.type == ActionType::APPLY_PROFILE) {
                char strBuf[256]; strncpy(strBuf, r.action.targetProfile.c_str(), sizeof(strBuf)); strBuf[sizeof(strBuf)-1] = 0;
                if (ImGui::InputText("Profile Name", strBuf, sizeof(strBuf))) r.action.targetProfile = strBuf;
            } else if (r.action.type == ActionType::SET_PROCESS_PRIORITY) {
                int prio = r.action.priorityClass;
                if (ImGui::InputInt("Priority Class", &prio)) r.action.priorityClass = prio;
            }
            ImGui::TreePop();
        }
    }
    if (ImGui::Button("Add Rule")) {
        Rule r; r.name = "New Rule " + std::to_string(rules.size() + 1);
        r.condition.type = ConditionType::CPU_USAGE_GREATER_THAN;
        r.action.type = ActionType::SET_PROCESS_PRIORITY;
        rules.push_back(r);
    }
}
void UI::RenderSessionAnalysis() {
    ImGui::Text("Session Analysis");
    ImGui::Separator();
    auto stats = SessionRecorder::Get().GetLastSessionStats();
    if (stats.durationSeconds > 0) {
        ImGui::Text("Game: %s", stats.gameName.c_str());
        ImGui::Text("Duration: %.1f s", stats.durationSeconds);
        ImGui::Text("Average FPS: %.1f", stats.averageFps);
        ImGui::Text("1%% Low: %.1f", stats.onePercentLowFps);
        ImGui::Text("0.1%% Low: %.1f", stats.zeroOnePercentLowFps);
        ImGui::Text("Stutter Events: %d", stats.stutterEvents);
    } else {
        ImGui::Text("No session recorded yet.");
    }
}

void UI::RenderBenchmarkTab() {
    ImGui::Text("Benchmark Runs");
    ImGui::Separator();

    std::wstring wGameName = GameDetector::Get().GetCurrentGameName();
    std::string gameName = "None";
    if (!wGameName.empty()) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        gameName = converter.to_bytes(wGameName);
    }

    if (BenchmarkManager::Get().IsRunning()) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Benchmark running...");
        if (ImGui::Button("Stop Benchmark")) {
            BenchmarkManager::Get().EndBenchmark();
        }
    } else {
        static char targetProfile[256] = "Default";
        ImGui::InputText("Profile to Benchmark", targetProfile, sizeof(targetProfile));
        if (ImGui::Button("Start Benchmark")) {
            BenchmarkManager::Get().StartBenchmark(gameName, targetProfile);
        }
    }

    ImGui::Separator();
    ImGui::Text("Results:");
    if (ImGui::Button("Clear Runs")) {
        BenchmarkManager::Get().ClearRuns();
    }

    const auto& runs = BenchmarkManager::Get().GetRuns();
    if (ImGui::BeginTable("BenchmarkResults", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Profile");
        ImGui::TableSetupColumn("Avg FPS");
        ImGui::TableSetupColumn("1% Low");
        ImGui::TableSetupColumn("0.1% Low");
        ImGui::TableSetupColumn("Stutters");
        ImGui::TableHeadersRow();

        for (const auto& run : runs) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("%s", run.profileName.c_str());
            ImGui::TableSetColumnIndex(1); ImGui::Text("%.1f", run.stats.averageFps);
            ImGui::TableSetColumnIndex(2); ImGui::Text("%.1f", run.stats.onePercentLowFps);
            ImGui::TableSetColumnIndex(3); ImGui::Text("%.1f", run.stats.zeroOnePercentLowFps);
            ImGui::TableSetColumnIndex(4); ImGui::Text("%d", run.stats.stutterEvents);
        }
        ImGui::EndTable();
    }
}
