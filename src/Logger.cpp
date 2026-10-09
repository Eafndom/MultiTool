#include "Logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>
void Logger::Init(const std::string& filename) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_file.is_open()) m_file.close();
    m_file.open(filename, std::ios::app);
}
void Logger::Log(Level level, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string levelStr;
    switch (level) {
        case Level::INFO: levelStr = "[INFO]"; break;
        case Level::WARN: levelStr = "[WARN]"; break;
        case Level::ERR: levelStr = "[ERROR]"; break;
    }
    std::string logLine = GetTimestamp() + " " + levelStr + " " + message;
    if (m_file.is_open()) m_file << logLine << std::endl;
    std::cout << logLine << std::endl;
}
std::string Logger::GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), "%H:%M:%S");
    return ss.str();
}
