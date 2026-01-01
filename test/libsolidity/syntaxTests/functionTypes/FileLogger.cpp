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

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
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
        if (!logFile.is_open()) return;

        logFile.close();

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
        fs::rename(logFilePath, firstBackup);

        logFile.open(logFilePath, std::ios::app);
    }

public:
    FileLogger(const std::string& filePath, LogLevel level = LogLevel::INFO,
               size_t maxSize = 1048576, int maxBackups = 5)
        : logFilePath(filePath), currentLevel(level),
          maxFileSize(maxSize), maxBackupFiles(maxBackups) {
        
        logFile.open(logFilePath, std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Cannot open log file: " + filePath);
        }
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(LogLevel level, const std::string& message) {
        if (level < currentLevel) return;

        std::lock_guard<std::mutex> lock(logMutex);

        if (logFile.tellp() > maxFileSize) {
            rotateLogs();
        }

        logFile << "[" << getTimestamp() << "] "
                << "[" << levelToString(level) << "] "
                << message << std::endl;
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

    void setLogLevel(LogLevel level) {
        currentLevel = level;
    }
};

void exampleUsage() {
    try {
        FileLogger logger("application.log", LogLevel::DEBUG);
        
        logger.debug("Application started");
        logger.info("Loading configuration");
        logger.warning("Deprecated API called");
        logger.error("Failed to connect to database");
        
        for (int i = 0; i < 1000; ++i) {
            logger.info("Processing item " + std::to_string(i));
        }
        
        logger.setLogLevel(LogLevel::WARNING);
        logger.debug("This debug message won't be logged");
        logger.warning("This warning will be logged");
        
    } catch (const std::exception& e) {
        std::cerr << "Logger error: " << e.what() << std::endl;
    }
}