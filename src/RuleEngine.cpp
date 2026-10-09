#include "RuleEngine.h"
#include "SystemControl.h"
#include "StateRestorer.h"
#include "Logger.h"
#include "GameDetector.h"
void RuleEngine::Evaluate(const SystemMetrics& metrics, const std::string& currentGameName) {
    for (auto& rule : rules) {
        if (!rule.enabled) { rule.active = false; continue; }
        bool conditionMet = EvaluateCondition(rule.condition, metrics, currentGameName);
        if (conditionMet && !rule.active) {
            rule.active = true;
            ExecuteAction(rule.action);
        } else if (!conditionMet && rule.active) {
            rule.active = false;
        }
    }
}
bool RuleEngine::EvaluateCondition(const RuleCondition& cond, const SystemMetrics& metrics, const std::string& currentGameName) {
    switch (cond.type) {
        case ConditionType::CPU_USAGE_GREATER_THAN: return metrics.cpuUtilization > cond.thresholdValue;
        case ConditionType::RAM_USAGE_GREATER_THAN: return metrics.ramUsageMB > cond.thresholdValue;
        case ConditionType::GAME_PROCESS_RUNNING: return currentGameName == cond.stringValue && !currentGameName.empty();
        default: return false;
    }
}
void RuleEngine::ExecuteAction(const RuleAction& action) {
    DWORD currentPid = GameDetector::Get().GetCurrentGamePID();
    switch (action.type) {
        case ActionType::APPLY_PROFILE: {
            auto profileOpt = DataManager::Get().GetProfileByName(action.targetProfile);
            if (profileOpt) {
                const auto& p = profileOpt.value();
                if (currentPid != 0) {
                    DWORD origPrio = SystemControl::Get().GetProcessPriority(currentPid);
                    DWORD_PTR origAffinity = SystemControl::Get().GetProcessAffinity(currentPid);
                    StateRestorer::Get().RegisterModifiedProcess(currentPid, origPrio, origAffinity);
                    SystemControl::Get().SetProcessPriority(currentPid, p.processPriority);
                    if (p.processAffinity != 0) SystemControl::Get().SetProcessAffinity(currentPid, p.processAffinity);
                }
                if (p.displayWidth > 0 && p.displayHeight > 0 && p.displayRefreshRate > 0) {
                    SystemControl::Get().ChangeDisplaySettings(p.displayWidth, p.displayHeight, p.displayRefreshRate);
                }
                if (!p.powerPlanGuid.empty()) {
                    GUID guid;
                    if (SystemControl::ParseGuid(p.powerPlanGuid, guid)) SystemControl::Get().SetActivePowerPlan(guid);
                }
            }
            break;
        }
        case ActionType::SET_PROCESS_PRIORITY: {
            if (currentPid != 0) {
                DWORD origPrio = SystemControl::Get().GetProcessPriority(currentPid);
                DWORD_PTR origAffinity = SystemControl::Get().GetProcessAffinity(currentPid);
                StateRestorer::Get().RegisterModifiedProcess(currentPid, origPrio, origAffinity);
                SystemControl::Get().SetProcessPriority(currentPid, action.priorityClass);
            }
            break;
        }
    }
}
