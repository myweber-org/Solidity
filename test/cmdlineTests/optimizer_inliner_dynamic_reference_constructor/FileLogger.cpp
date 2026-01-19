#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>
#include <mutex>
#include <sstream>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

class FileLogger {
private:
    std::ofstream logFile;
    std::mutex logMutex;
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&time);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
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
    FileLogger(const std::string& filename) {
        logFile.open(filename, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file: " << filename << std::endl;
        }
    }
    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }
    void log(LogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        if (logFile.is_open()) {
            logFile << "[" << getCurrentTimestamp() << "] "
                    << "[" << levelToString(level) << "] "
                    << message << std::endl;
        }
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

int main() {
    FileLogger logger("application.log");
    logger.info("Application started successfully");
    logger.warning("Disk space is running low");
    logger.error("Failed to connect to database");
    return 0;
}