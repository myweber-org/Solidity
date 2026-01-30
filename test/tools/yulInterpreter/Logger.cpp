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

class Logger {
private:
    std::ofstream logFile;
    std::string logFileName;
    std::string logDirectory;
    size_t maxFileSize;
    int maxBackupFiles;
    LogLevel currentLevel;
    std::mutex logMutex;

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()
        ) % 1000;

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

        logFile.flush();
        logFile.close();

        for (int i = maxBackupFiles - 1; i >= 0; --i) {
            fs::path oldPath;
            if (i == 0) {
                oldPath = fs::path(logDirectory) / logFileName;
            } else {
                oldPath = fs::path(logDirectory) / (logFileName + "." + std::to_string(i));
            }

            fs::path newPath = fs::path(logDirectory) / (logFileName + "." + std::to_string(i + 1));

            if (fs::exists(oldPath)) {
                if (i == maxBackupFiles - 1) {
                    fs::remove(oldPath);
                } else {
                    fs::rename(oldPath, newPath);
                }
            }
        }

        openLogFile();
    }

    void openLogFile() {
        fs::path fullPath = fs::path(logDirectory) / logFileName;
        logFile.open(fullPath, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file: " << fullPath << std::endl;
        }
    }

public:
    Logger(const std::string& dir = "logs", 
           const std::string& filename = "application.log",
           size_t maxSize = 1048576, // 1MB
           int backups = 5,
           LogLevel level = LogLevel::INFO)
        : logDirectory(dir), logFileName(filename), 
          maxFileSize(maxSize), maxBackupFiles(backups),
          currentLevel(level) {
        
        if (!fs::exists(logDirectory)) {
            fs::create_directories(logDirectory);
        }
        
        openLogFile();
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

        if (!logFile.is_open()) {
            std::cerr << "Log file is not open" << std::endl;
            return;
        }

        std::string timestamp = getTimestamp();
        std::string levelStr = levelToString(level);
        
        std::string logEntry = "[" + timestamp + "] [" + levelStr + "] " + message + "\n";

        logFile << logEntry;
        logFile.flush();

        if (logFile.tellp() > maxFileSize) {
            rotateLogs();
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
    Logger logger("logs", "app.log", 1024, 3, LogLevel::DEBUG);
    
    logger.debug("This is a debug message");
    logger.info("Application started successfully");
    logger.warning("Disk space is running low");
    logger.error("Failed to connect to database");
    
    for(int i = 0; i < 100; i++) {
        logger.info("Log entry number " + std::to_string(i));
    }
}

int main() {
    exampleUsage();
    return 0;
}