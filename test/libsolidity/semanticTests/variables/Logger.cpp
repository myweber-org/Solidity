
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
    LogLevel currentLevel;
    std::string filename;
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
            std::string oldName = filename + "." + std::to_string(i);
            std::string newName = filename + "." + std::to_string(i + 1);
            
            if (std::filesystem::exists(oldName)) {
                std::filesystem::rename(oldName, newName);
            }
        }
        
        std::string firstBackup = filename + ".1";
        std::filesystem::rename(filename, firstBackup);
        
        logFile.open(filename, std::ios::out | std::ios::app);
    }

public:
    Logger(const std::string& file = "app.log", 
           LogLevel level = LogLevel::INFO,
           size_t maxSize = 1048576, // 1MB
           int maxBackups = 5)
        : filename(file), currentLevel(level), 
          maxFileSize(maxSize), maxBackupFiles(maxBackups) {
        
        logFile.open(filename, std::ios::out | std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file: " << filename << std::endl;
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

        std::string timestamp = getTimestamp();
        std::string levelStr = levelToString(level);
        
        logFile << "[" << timestamp << "] [" << levelStr << "] " << message << std::endl;
        
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

void exampleUsage() {
    Logger logger("application.log", LogLevel::DEBUG);
    
    logger.debug("Application started");
    logger.info("Loading configuration");
    logger.warning("Deprecated API called");
    logger.error("Failed to connect to database");
    
    logger.setLogLevel(LogLevel::WARNING);
    logger.debug("This debug message won't be logged");
    logger.error("Critical error occurred");
}

int main() {
    exampleUsage();
    return 0;
}