
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
    ERROR,
    CRITICAL
};

class Logger {
private:
    std::ofstream logFile;
    std::string logFileName;
    LogLevel currentLevel;
    std::mutex logMutex;
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
        
        for (int i = maxBackupFiles - 1; i > 0; --i) {
            std::string oldName = logFileName + "." + std::to_string(i);
            std::string newName = logFileName + "." + std::to_string(i + 1);
            
            if (std::filesystem::exists(oldName)) {
                std::filesystem::rename(oldName, newName);
            }
        }
        
        std::string firstBackup = logFileName + ".1";
        std::filesystem::rename(logFileName, firstBackup);
        
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

        std::string timestamp = getTimestamp();
        std::string levelStr = levelToString(level);
        
        logFile << "[" << timestamp << "] [" << levelStr << "] " << message << std::endl;
        
        if (level >= LogLevel::ERROR) {
            std::cerr << "[" << timestamp << "] [" << levelStr << "] " << message << std::endl;
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

void exampleUsage() {
    Logger logger("myapp.log", LogLevel::DEBUG);
    
    logger.debug("Application started");
    logger.info("User 'admin' logged in");
    logger.warning("Disk space is running low");
    logger.error("Failed to connect to database");
    logger.critical("System shutdown due to critical error");
    
    logger.setLogLevel(LogLevel::WARNING);
    logger.debug("This debug message won't be logged");
    logger.info("This info message won't be logged");
    logger.warning("This warning will be logged");
}