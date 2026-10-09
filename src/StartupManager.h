#pragma once

class StartupManager {
public:
    static bool SetRunAtStartup(bool enable);
    static bool IsRunAtStartupEnabled();
};
