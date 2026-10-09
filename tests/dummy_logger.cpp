#include "Logger.h"
#include <iostream>
void Logger::Init(const std::string&) {}
void Logger::Log(Level, const std::string& msg) { std::cout << msg << "\n"; }
std::string Logger::GetTimestamp() { return ""; }
