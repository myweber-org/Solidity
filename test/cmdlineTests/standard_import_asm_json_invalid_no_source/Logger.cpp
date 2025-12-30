#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>

class Logger {
public:
    enum class LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    Logger(const std::string& filename, LogLevel minLevel = LogLevel::INFO)
        : logFile(filename, std::ios::app), minimumLevel(minLevel) {
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
        if (level < minimumLevel || !logFile.is_open()) {
            return;
        }

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        logFile << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        logFile << "." << std::setfill('0') << std::setw(3) << ms.count() << "] ";
        logFile << levelToString(level) << ": " << message << std::endl;
    }

    void debug(const std::string& message) {
        log(LogLevel::DEBUG, message);
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

private:
    std::ofstream logFile;
    LogLevel minimumLevel;

    std::string levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG:   return "DEBUG";
            case LogLevel::INFO:    return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR:   return "ERROR";
            default:                return "UNKNOWN";
        }
    }
};

void exampleUsage() {
    Logger logger("application.log", Logger::LogLevel::DEBUG);
    
    logger.debug("Starting application initialization");
    logger.info("Application started successfully");
    logger.warning("Configuration file not found, using defaults");
    logger.error("Failed to connect to database");
    
    int value = 42;
    logger.debug("Current value: " + std::to_string(value));
}