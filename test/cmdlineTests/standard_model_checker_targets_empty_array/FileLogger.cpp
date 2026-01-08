#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <mutex>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class FileLogger {
public:
    FileLogger(const std::string& filename, LogLevel minLevel = LogLevel::INFO)
        : logFile(filename, std::ios::app), minimumLevel(minLevel) {
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
        if (level < minimumLevel) return;

        std::lock_guard<std::mutex> lock(logMutex);
        if (!logFile.is_open()) return;

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        logFile << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        logFile << '.' << std::setfill('0') << std::setw(3) << ms.count();
        logFile << " [" << levelToString(level) << "] ";
        logFile << message << std::endl;
    }

    void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    void info(const std::string& message) { log(LogLevel::INFO, message); }
    void warning(const std::string& message) { log(LogLevel::WARNING, message); }
    void error(const std::string& message) { log(LogLevel::ERROR, message); }

    void setMinLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(logMutex);
        minimumLevel = level;
    }

private:
    std::string levelToString(LogLevel level) {
        switch(level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

    std::ofstream logFile;
    LogLevel minimumLevel;
    std::mutex logMutex;
};

void exampleUsage() {
    FileLogger logger("application.log", LogLevel::DEBUG);
    
    logger.debug("Starting application initialization");
    logger.info("Application started successfully");
    logger.warning("Resource usage is above 80%");
    logger.error("Failed to connect to database");
    
    logger.setMinLevel(LogLevel::WARNING);
    logger.debug("This debug message will not be logged");
    logger.info("This info message will not be logged");
    logger.warning("This warning will be logged");
}