#pragma once
#include <string>
#include <mutex>
#include <fstream>
class Logger {
public:
    enum class Level { INFO, WARN, ERR };
    static Logger& Get() { static Logger instance; return instance; }
    void Init(const std::string& filename);
    void Log(Level level, const std::string& message);
    void Info(const std::string& message) { Log(Level::INFO, message); }
    void Warn(const std::string& message) { Log(Level::WARN, message); }
    void Error(const std::string& message) { Log(Level::ERR, message); }
private:
    Logger() = default;
    ~Logger() { if (m_file.is_open()) m_file.close(); }
    std::string GetTimestamp();
    std::mutex m_mutex;
    std::ofstream m_file;
};
