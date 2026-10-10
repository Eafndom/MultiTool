#pragma once
#include <string>
#include <vector>
#include <optional>
#ifdef _WIN32
#include <windows.h>
#else
#include "dummy_windows.h"
#endif
#include <nlohmann/json.hpp>
struct Profile {
    std::string name;
    std::string powerPlanGuid;
    DWORD processPriority = NORMAL_PRIORITY_CLASS;
    DWORD_PTR processAffinity = 0;
    int displayWidth = 0;
    int displayHeight = 0;
    int displayRefreshRate = 0;
    double targetFPS = 0.0;
};
struct GameSettings {
    std::string processName;
    std::string profileName;
    bool autoApply = true;
};
class DataManager {
public:
    static DataManager& Get() { static DataManager instance; return instance; }
    void Load();
    void Save();
    std::vector<Profile> profiles;
    std::vector<GameSettings> games;
    std::optional<Profile> GetProfileByName(const std::string& name) const;
    std::optional<GameSettings> GetGameByProcessName(const std::string& processName) const;
private:
    DataManager() = default;
    nlohmann::json m_data;
    const std::string m_filename = "gpcc_data.json";
};
