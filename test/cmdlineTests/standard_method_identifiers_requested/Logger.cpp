#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>
#include <sstream>
#include <mutex>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

class Logger {
private:
    std::ofstream logFile;
    std::mutex logMutex;
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm* local_time = std::localtime(&now_time);
        std::ostringstream oss;
        oss << std::put_time(local_time, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
    std::string levelToString(LogLevel level) {
        switch(level) {
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }
public:
    Logger(const std::string& filename) {
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
    void log(LogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        std::string logEntry = "[" + getCurrentTimestamp() + "] [" + levelToString(level) + "] " + message;
        if (logFile.is_open()) {
            logFile << logEntry << std::endl;
        }
        std::cout << logEntry << std::endl;
    }
    void info(const std::string& message) {
        log(LogLevel::INFO, message);
    }
    void warning(const std::string& message) {
        log(LogLevel::WARNING, message);
    }
    void error(const std::string& message) {
        log(LogLevel::ERROR, message);
    }
};