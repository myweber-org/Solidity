
#include <fstream>
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <mutex>

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
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::tm tm_now = *std::localtime(&time_t_now);
        std::ostringstream oss;
        oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
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
    logger.info("Application started successfully.");
    logger.warning("Disk space is running low.");
    logger.error("Failed to connect to database.");
    logger.info("Application shutdown initiated.");
    return 0;
}