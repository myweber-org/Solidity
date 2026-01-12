
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <mutex>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
private:
    std::ofstream logFile;
    std::string logFileName;
    LogLevel currentLevel;
    std::mutex logMutex;
    size_t maxFileSize;
    int maxBackupFiles;

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

    void rotateLogFile() {
        if (!logFile.is_open()) return;

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
           size_t maxSize = 1048576, // 1MB
           int maxBackups = 5)
        : logFileName(filename), currentLevel(level), 
          maxFileSize(maxSize), maxBackupFiles(maxBackups) {
        
        logFile.open(logFileName, std::ios::out | std::ios::app);
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

        std::string timestamp = getCurrentTimestamp();
        std::string levelStr = levelToString(level);
        
        std::string logEntry = "[" + timestamp + "] [" + levelStr + "] " + message + "\n";
        
        logFile << logEntry;
        logFile.flush();
        
        std::cout << logEntry;
        
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
};