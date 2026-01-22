
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <mutex>

namespace fs = std::filesystem;

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

class Logger {
private:
    std::ofstream logFile;
    LogLevel currentLevel;
    std::string logFilePath;
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
            case LogLevel::CRITICAL: return "CRITICAL";
            default: return "UNKNOWN";
        }
    }

    void rotateLogs() {
        if (!fs::exists(logFilePath)) return;

        auto fileSize = fs::file_size(logFilePath);
        if (fileSize < maxFileSize) return;

        std::lock_guard<std::mutex> lock(logMutex);
        logFile.close();

        for (int i = maxBackupFiles - 1; i > 0; --i) {
            std::string oldFile = logFilePath + "." + std::to_string(i);
            std::string newFile = logFilePath + "." + std::to_string(i + 1);
            
            if (fs::exists(oldFile)) {
                if (fs::exists(newFile)) {
                    fs::remove(newFile);
                }
                fs::rename(oldFile, newFile);
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
    Logger(const std::string& filePath = "application.log", 
           LogLevel level = LogLevel::INFO,
           size_t maxSize = 10485760,
           int maxBackups = 5)
        : logFilePath(filePath), currentLevel(level), 
          maxFileSize(maxSize), maxBackupFiles(maxBackups) {
        
        logFile.open(logFilePath, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file: " << logFilePath << std::endl;
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

        std::lock_guard<std::mutex> lock(logMutex);
        
        if (!logFile.is_open()) return;

        std::string logEntry = "[" + getTimestamp() + "] ";
        logEntry += "[" + levelToString(level) + "] ";
        logEntry += message;

        logFile << logEntry << std::endl;
        logFile.flush();

        if (level >= LogLevel::WARNING) {
            std::cerr << logEntry << std::endl;
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

    void critical(const std::string& message) {
        log(LogLevel::CRITICAL, message);
    }
};