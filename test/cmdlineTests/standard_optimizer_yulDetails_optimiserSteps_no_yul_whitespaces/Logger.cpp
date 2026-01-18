
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <queue>

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
    size_t maxFileSize;
    int maxBackupFiles;
    std::queue<std::string> backupFiles;

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
        if (logFile.is_open()) {
            logFile.close();
        }

        for (int i = maxBackupFiles - 1; i > 0; --i) {
            std::string oldName = logFileName + "." + std::to_string(i);
            std::string newName = logFileName + "." + std::to_string(i + 1);
            
            if (fs::exists(oldName)) {
                fs::rename(oldName, newName);
            }
        }

        std::string firstBackup = logFileName + ".1";
        fs::rename(logFileName, firstBackup);
        backupFiles.push(firstBackup);

        while (backupFiles.size() > maxBackupFiles) {
            std::string oldBackup = backupFiles.front();
            backupFiles.pop();
            if (fs::exists(oldBackup)) {
                fs::remove(oldBackup);
            }
        }

        logFile.open(logFileName, std::ios::app);
    }

public:
    Logger(const std::string& filename = "app.log", 
           LogLevel level = LogLevel::INFO,
           size_t maxSize = 1048576, // 1MB
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
        currentLevel = level;
    }

    void log(LogLevel level, const std::string& message) {
        if (level < currentLevel) {
            return;
        }

        if (logFile.tellp() > maxFileSize) {
            rotateLogFile();
        }

        std::string timestamp = getTimestamp();
        std::string levelStr = levelToString(level);
        
        std::string logEntry = "[" + timestamp + "] [" + levelStr + "] " + message + "\n";
        
        logFile << logEntry;
        logFile.flush();

        std::cout << logEntry;
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

    void clearBackups() {
        while (!backupFiles.empty()) {
            std::string backup = backupFiles.front();
            backupFiles.pop();
            if (fs::exists(backup)) {
                fs::remove(backup);
            }
        }

        for (int i = 1; i <= maxBackupFiles; ++i) {
            std::string backupName = logFileName + "." + std::to_string(i);
            if (fs::exists(backupName)) {
                fs::remove(backupName);
            }
        }
    }
};

void exampleUsage() {
    Logger logger("application.log", LogLevel::DEBUG);
    
    logger.debug("Debug message for detailed information");
    logger.info("Application started successfully");
    logger.warning("Disk space is running low");
    logger.error("Failed to connect to database");
    logger.critical("System shutdown due to critical error");
    
    logger.setLogLevel(LogLevel::WARNING);
    logger.debug("This debug message will not be logged");
    logger.info("This info message will not be logged");
    logger.warning("This warning will be logged");
    
    logger.clearBackups();
}

int main() {
    exampleUsage();
    return 0;
}