#pragma once
#include <string>
#include <vector>
#include "PerformanceMonitor.h"
#include "DataManager.h"
enum class ConditionType { CPU_USAGE_GREATER_THAN, RAM_USAGE_GREATER_THAN, GAME_PROCESS_RUNNING };
enum class ActionType { APPLY_PROFILE, SET_PROCESS_PRIORITY };
struct RuleCondition { ConditionType type; double thresholdValue = 0.0; std::string stringValue = ""; };
struct RuleAction { ActionType type; std::string targetProfile = ""; DWORD priorityClass = 0; };
struct Rule { std::string name; RuleCondition condition; RuleAction action; bool enabled = true; bool active = false; };
class RuleEngine {
public:
    static RuleEngine& Get() { static RuleEngine instance; return instance; }
    void Evaluate(const SystemMetrics& metrics, const std::string& currentGameName);
    void ExecuteAction(const RuleAction& action);
    std::vector<Rule> rules;
private:
    RuleEngine() = default;
    bool EvaluateCondition(const RuleCondition& cond, const SystemMetrics& metrics, const std::string& currentGameName);
};
