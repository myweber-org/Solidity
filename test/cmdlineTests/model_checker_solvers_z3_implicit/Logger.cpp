#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

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
    std::string logFilePath;
    size_t maxFileSize;
    int maxBackupFiles;

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
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

    void rotateLogs() {
        if (!fs::exists(logFilePath)) return;

        auto fileSize = fs::file_size(logFilePath);
        if (fileSize < maxFileSize) return;

        for (int i = maxBackupFiles - 1; i > 0; --i) {
            std::string oldName = logFilePath + "." + std::to_string(i);
            std::string newName = logFilePath + "." + std::to_string(i + 1);
            
            if (fs::exists(oldName)) {
                if (fs::exists(newName)) {
                    fs::remove(newName);
                }
                fs::rename(oldName, newName);
            }
        }

        std::string firstBackup = logFilePath + ".1";
        if (fs::exists(firstBackup)) {
            fs::remove(firstBackup);
        }
        
        logFile.close();
        fs::rename(logFilePath, firstBackup);
        
        logFile.open(logFilePath, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Failed to reopen log file after rotation" << std::endl;
        }
    }

public:
    Logger(const std::string& filePath, LogLevel level = LogLevel::INFO, 
           size_t maxSize = 1048576, int maxBackups = 5)
        : logFilePath(filePath), currentLevel(level), 
          maxFileSize(maxSize), maxBackupFiles(maxBackups) {
        
        logFile.open(filePath, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file: " << filePath << std::endl;
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

        rotateLogs();

        if (logFile.is_open()) {
            logFile << "[" << getTimestamp() << "] "
                    << "[" << levelToString(level) << "] "
                    << message << std::endl;
        }

        if (level >= LogLevel::WARNING) {
            std::cerr << "[" << getTimestamp() << "] "
                      << "[" << levelToString(level) << "] "
                      << message << std::endl;
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
    Logger logger("application.log", LogLevel::DEBUG);
    
    logger.debug("Starting application initialization");
    logger.info("Application started successfully");
    logger.warning("Configuration file not found, using defaults");
    logger.error("Failed to connect to database");
    
    for (int i = 0; i < 1000; ++i) {
        logger.info("Log entry number: " + std::to_string(i));
    }
}

int main() {
    exampleUsage();
    return 0;
}