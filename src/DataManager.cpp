#include "DataManager.h"
#include "Logger.h"
#include <fstream>
using json = nlohmann::json;
void to_json(json& j, const Profile& p) {
    j = json{{"name", p.name}, {"powerPlanGuid", p.powerPlanGuid}, {"processPriority", p.processPriority},
             {"processAffinity", p.processAffinity}, {"displayWidth", p.displayWidth},
             {"displayHeight", p.displayHeight}, {"displayRefreshRate", p.displayRefreshRate},
             {"targetFPS", p.targetFPS}};
}
void from_json(const json& j, Profile& p) {
    j.at("name").get_to(p.name); j.at("powerPlanGuid").get_to(p.powerPlanGuid);
    j.at("processPriority").get_to(p.processPriority); j.at("processAffinity").get_to(p.processAffinity);
    j.at("displayWidth").get_to(p.displayWidth); j.at("displayHeight").get_to(p.displayHeight);
    j.at("displayRefreshRate").get_to(p.displayRefreshRate); j.at("targetFPS").get_to(p.targetFPS);
}
void to_json(json& j, const GameSettings& g) {
    j = json{{"processName", g.processName}, {"profileName", g.profileName}, {"autoApply", g.autoApply}};
}
void from_json(const json& j, GameSettings& g) {
    j.at("processName").get_to(g.processName); j.at("profileName").get_to(g.profileName); j.at("autoApply").get_to(g.autoApply);
}
void DataManager::Load() {
    std::ifstream file(m_filename);
    if (!file.is_open()) {
        Profile defaultProfile{"Default", "", NORMAL_PRIORITY_CLASS, 0, 0, 0, 0, 60.0};
        profiles.push_back(defaultProfile);
        Save();
        return;
    }
    try {
        file >> m_data;
        if (m_data.contains("profiles")) profiles = m_data["profiles"].get<std::vector<Profile>>();
        if (m_data.contains("games")) games = m_data["games"].get<std::vector<GameSettings>>();
    } catch (...) {}
}
void DataManager::Save() {
    m_data["profiles"] = profiles; m_data["games"] = games;
    std::ofstream file(m_filename);
    if (file.is_open()) file << m_data.dump(4);
}
std::optional<Profile> DataManager::GetProfileByName(const std::string& name) const {
    for (const auto& p : profiles) if (p.name == name) return p;
    return std::nullopt;
}
std::optional<GameSettings> DataManager::GetGameByProcessName(const std::string& processName) const {
    for (const auto& g : games) if (g.processName == processName) return g;
    return std::nullopt;
}
