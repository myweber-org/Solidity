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
private:
    std::ofstream logFile;
    LogLevel currentLevel;
    std::mutex logMutex;
    std::string levelToString(LogLevel level) {
        switch(level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

public:
    FileLogger(const std::string& filename, LogLevel level = LogLevel::INFO) 
        : currentLevel(level) {
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
    
    void setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(logMutex);
        currentLevel = level;
    }
    
    void log(LogLevel level, const std::string& message) {
        if (level < currentLevel) return;
        
        std::lock_guard<std::mutex> lock(logMutex);
        if (!logFile.is_open()) return;
        
        logFile << "[" << getCurrentTimestamp() << "] "
                << "[" << levelToString(level) << "] "
                << message << std::endl;
        
        logFile.flush();
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
    FileLogger logger("application.log", LogLevel::DEBUG);
    
    logger.debug("Starting application initialization");
    logger.info("Application started successfully");
    logger.warning("Configuration file not found, using defaults");
    logger.error("Failed to connect to database");
    
    logger.setLogLevel(LogLevel::WARNING);
    logger.debug("This debug message will not be logged");
    logger.info("This info message will not be logged");
    logger.warning("This warning will be logged");
}