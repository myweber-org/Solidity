
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <mutex>

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
    std::string logFileName;
    std::mutex logMutex;
    LogLevel currentLevel;
    size_t maxFileSize;
    int maxBackupFiles;

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

    void rotateLogFile() {
        logFile.close();
        
        for (int i = maxBackupFiles - 1; i > 0; --i) {
            std::string oldName = logFileName + "." + std::to_string(i);
            std::string newName = logFileName + "." + std::to_string(i + 1);
            
            if (std::filesystem::exists(oldName)) {
                std::filesystem::rename(oldName, newName);
            }
        }
        
        std::string firstBackup = logFileName + ".1";
        std::filesystem::rename(logFileName, firstBackup);
        
        logFile.open(logFileName, std::ios::out | std::ios::app);
    }

public:
    Logger(const std::string& filename = "application.log", 
           LogLevel level = LogLevel::INFO,
           size_t maxSize = 10485760, // 10MB
           int maxBackups = 5)
        : logFileName(filename), currentLevel(level), 
          maxFileSize(maxSize), maxBackupFiles(maxBackups) {
        
        logFile.open(logFileName, std::ios::out | std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Failed to open log file: " + filename);
        }
    }

    ~Logger() {
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

        std::string logEntry = "[" + getTimestamp() + "] [" + 
                              levelToString(level) + "] " + message + "\n";

        logFile << logEntry;
        logFile.flush();

        if (logFile.tellp() > maxFileSize) {
            rotateLogFile();
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