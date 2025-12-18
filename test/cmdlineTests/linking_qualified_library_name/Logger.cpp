
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

    void rotateLogs() {
        if (!logFile.is_open()) return;

        logFile.close();

        for (int i = maxBackupFiles - 1; i >= 0; --i) {
            std::string oldName = i == 0 ? logFileName : logFileName + "." + std::to_string(i);
            std::string newName = logFileName + "." + std::to_string(i + 1);

            if (fs::exists(oldName)) {
                if (i == maxBackupFiles - 1) {
                    fs::remove(oldName);
                } else {
                    fs::rename(oldName, newName);
                }
            }
        }

        if (fs::exists(logFileName)) {
            fs::rename(logFileName, logFileName + ".1");
        }

        logFile.open(logFileName, std::ios::app);
    }

public:
    Logger(const std::string& filename = "application.log", 
           LogLevel level = LogLevel::INFO,
           size_t maxSize = 10485760, // 10MB
           int maxBackups = 5)
        : logFileName(filename), currentLevel(level), 
          maxFileSize(maxSize), maxBackupFiles(maxBackups) {
        
        logFile.open(logFileName, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file: " << logFileName << std::endl;
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

        if (logFile.tellp() > maxFileSize) {
            rotateLogs();
        }

        logFile << "[" << getTimestamp() << "] "
                << "[" << levelToString(level) << "] "
                << message << std::endl;

        if (level >= LogLevel::ERROR) {
            std::cerr << "[" << levelToString(level) << "] " << message << std::endl;
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