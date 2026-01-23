
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    Logger(const std::string& filename, Level minLevel = Level::INFO) 
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

    void log(Level level, const std::string& message) {
        if (level < minimumLevel || !logFile.is_open()) {
            return;
        }

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();

        logFile << "[" << ss.str() << "] "
                << levelToString(level) << ": "
                << message << std::endl;
    }

    void debug(const std::string& message) {
        log(Level::DEBUG, message);
    }

    void info(const std::string& message) {
        log(Level::INFO, message);
    }

    void warning(const std::string& message) {
        log(Level::WARNING, message);
    }

    void error(const std::string& message) {
        log(Level::ERROR, message);
    }

    void setMinimumLevel(Level level) {
        minimumLevel = level;
    }

private:
    std::ofstream logFile;
    Level minimumLevel;

    std::string levelToString(Level level) {
        switch (level) {
            case Level::DEBUG: return "DEBUG";
            case Level::INFO: return "INFO";
            case Level::WARNING: return "WARNING";
            case Level::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }
};

void exampleUsage() {
    Logger logger("application.log", Logger::Level::DEBUG);
    
    logger.debug("Starting application initialization");
    logger.info("Application started successfully");
    logger.warning("Configuration file not found, using defaults");
    logger.error("Failed to connect to database");
    
    logger.setMinimumLevel(Logger::Level::WARNING);
    logger.debug("This debug message won't be logged");
    logger.info("This info message won't be logged");
    logger.warning("This warning will be logged");
}