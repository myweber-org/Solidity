
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
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
    bool outputToConsole;

    std::string getCurrentTime() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    std::string levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

public:
    Logger(const std::string& filename = "", LogLevel level = LogLevel::INFO, bool console = true)
        : currentLevel(level), outputToConsole(console) {
        if (!filename.empty()) {
            logFile.open(filename, std::ios::app);
        }
    }

    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void setLogLevel(LogLevel level) {
        currentLevel = level;
    }

    void log(LogLevel level, const std::string& message) {
        if (level < currentLevel) return;

        std::string logEntry = "[" + getCurrentTime() + "] [" + levelToString(level) + "] " + message;

        if (outputToConsole) {
            std::cout << logEntry << std::endl;
        }

        if (logFile.is_open()) {
            logFile << logEntry << std::endl;
        }
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
};

void exampleUsage() {
    Logger logger("application.log", LogLevel::DEBUG, true);
    
    logger.debug("This is a debug message");
    logger.info("Application started successfully");
    logger.warning("Resource usage is above 80%");
    logger.error("Failed to connect to database");
    
    logger.setLogLevel(LogLevel::WARNING);
    logger.debug("This debug message will not be logged");
    logger.warning("This warning will be logged");
}