
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

    void rotateLogIfNeeded() {
        if (!logFile.is_open()) return;

        logFile.flush();
        auto currentSize = static_cast<size_t>(logFile.tellp());
        
        if (currentSize >= maxFileSize) {
            logFile.close();
            
            for (int i = maxBackupFiles - 1; i > 0; --i) {
                fs::path oldFile = logFileName + "." + std::to_string(i);
                fs::path newFile = logFileName + "." + std::to_string(i + 1);
                
                if (fs::exists(oldFile)) {
                    fs::rename(oldFile, newFile);
                }
            }
            
            fs::path firstBackup = logFileName + ".1";
            if (fs::exists(logFileName)) {
                fs::rename(logFileName, firstBackup);
            }
            
            logFile.open(logFileName, std::ios::app);
        }
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
        
        rotateLogIfNeeded();
        
        std::string timestamp = getTimestamp();
        std::string levelStr = levelToString(level);
        
        std::string logEntry = "[" + timestamp + "] [" + levelStr + "] " + message;
        
        if (logFile.is_open()) {
            logFile << logEntry << std::endl;
            logFile.flush();
        }
        
        std::cout << logEntry << std::endl;
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
    
    logger.debug("Debug message for detailed information");
    logger.info("Application started successfully");
    logger.warning("Disk space is running low");
    logger.error("Failed to connect to database");
    logger.critical("System shutdown due to critical error");
    
    logger.setLogLevel(LogLevel::WARNING);
    logger.info("This info message won't be logged due to level filter");
    logger.error("This error will still be logged");
}

int main() {
    exampleUsage();
    return 0;
}