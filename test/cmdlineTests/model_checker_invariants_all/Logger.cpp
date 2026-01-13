
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <sstream>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
private:
    std::ofstream logFile;
    LogLevel currentLevel;
    std::string getCurrentTime() {
        std::time_t now = std::time(nullptr);
        std::tm* localTime = std::localtime(&now);
        std::ostringstream oss;
        oss << (localTime->tm_year + 1900) << '-'
            << (localTime->tm_mon + 1) << '-'
            << localTime->tm_mday << ' '
            << localTime->tm_hour << ':'
            << localTime->tm_min << ':'
            << localTime->tm_sec;
        return oss.str();
    }
    std::string levelToString(LogLevel level) {
        switch(level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }
public:
    Logger(const std::string& filename, LogLevel level = LogLevel::INFO) : currentLevel(level) {
        logFile.open(filename, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file: " << filename << std::endl;
        }
    }
    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }
    void setLevel(LogLevel level) {
        currentLevel = level;
    }
    void log(LogLevel level, const std::string& message) {
        if (level < currentLevel) return;
        std::string logEntry = "[" + getCurrentTime() + "] [" + levelToString(level) + "] " + message;
        std::cout << logEntry << std::endl;
        if (logFile.is_open()) {
            logFile << logEntry << std::endl;
        }
    }
    void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    void info(const std::string& message) { log(LogLevel::INFO, message); }
    void warning(const std::string& message) { log(LogLevel::WARNING, message); }
    void error(const std::string& message) { log(LogLevel::ERROR, message); }
};