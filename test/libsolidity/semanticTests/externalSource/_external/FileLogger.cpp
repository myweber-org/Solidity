#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <mutex>

namespace fs = std::filesystem;

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class FileLogger {
private:
    std::ofstream logFile;
    std::string logFilePath;
    LogLevel currentLevel;
    size_t maxFileSize;
    int maxBackupFiles;
    std::mutex logMutex;

    std::string getCurrentTimestamp() {
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

    void rotateLogIfNeeded() {
        if (!fs::exists(logFilePath)) return;
        
        size_t fileSize = fs::file_size(logFilePath);
        if (fileSize < maxFileSize) return;

        for (int i = maxBackupFiles - 1; i > 0; --i) {
            std::string oldFile = logFilePath + "." + std::to_string(i);
            std::string newFile = logFilePath + "." + std::to_string(i + 1);
            
            if (fs::exists(oldFile)) {
                fs::rename(oldFile, newFile);
            }
        }

        std::string firstBackup = logFilePath + ".1";
        fs::rename(logFilePath, firstBackup);
        
        logFile.close();
        logFile.open(logFilePath, std::ios::out | std::ios::app);
    }

public:
    FileLogger(const std::string& path, LogLevel level = LogLevel::INFO, 
               size_t maxSize = 1048576, int maxBackups = 5)
        : logFilePath(path), currentLevel(level), 
          maxFileSize(maxSize), maxBackupFiles(maxBackups) {
        
        fs::create_directories(fs::path(path).parent_path());
        logFile.open(path, std::ios::out | std::ios::app);
        
        if (!logFile.is_open()) {
            throw std::runtime_error("Cannot open log file: " + path);
        }
        
        log(LogLevel::INFO, "Logger initialized");
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            log(LogLevel::INFO, "Logger shutting down");
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
        
        rotateLogIfNeeded();
        
        logFile << "[" << getCurrentTimestamp() << "] "
                << "[" << levelToString(level) << "] "
                << message << std::endl;
        
        if (level >= LogLevel::ERROR) {
            logFile.flush();
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
    try {
        FileLogger logger("logs/application.log", LogLevel::DEBUG);
        
        logger.debug("This is a debug message");
        logger.info("Application started successfully");
        logger.warning("Disk space is running low");
        logger.error("Failed to connect to database");
        
        logger.setLogLevel(LogLevel::WARNING);
        logger.debug("This debug message won't be logged");
        logger.warning("This warning will be logged");
        
    } catch (const std::exception& e) {
        std::cerr << "Logger error: " << e.what() << std::endl;
    }
}