
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
    ERROR
};

class Logger {
private:
    std::ofstream logFile;
    LogLevel currentLevel;
    std::string logFileName;
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

    void rotateLogFile() {
        logFile.close();
        
        for (int i = maxBackupFiles - 1; i > 0; --i) {
            std::string oldName = logFileName + "." + std::to_string(i);
            std::string newName = logFileName + "." + std::to_string(i + 1);
            
            if (fs::exists(oldName)) {
                fs::rename(oldName, newName);
            }
        }
        
        if (fs::exists(logFileName)) {
            fs::rename(logFileName, logFileName + ".1");
        }
        
        logFile.open(logFileName, std::ios::out | std::ios::app);
    }

public:
    Logger(const std::string& filename = "app.log", 
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

        if (logFile.tellp() > maxFileSize) {
            rotateLogFile();
        }

        std::string timestamp = getTimestamp();
        std::string levelStr = levelToString(level);
        
        logFile << "[" << timestamp << "] [" << levelStr << "] " << message << std::endl;
        logFile.flush();

        std::cout << "[" << timestamp << "] [" << levelStr << "] " << message << std::endl;
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
    
    logger.debug("Starting application");
    logger.info("Application initialized successfully");
    logger.warning("Low disk space detected");
    logger.error("Failed to connect to database");
    
    logger.setLogLevel(LogLevel::WARNING);
    logger.debug("This debug message will not be logged");
    logger.info("This info message will not be logged");
    logger.warning("This warning will be logged");
}

int main() {
    exampleUsage();
    return 0;
}