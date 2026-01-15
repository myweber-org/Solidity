
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
    ERROR
};

class FileLogger {
private:
    std::ofstream logFile;
    std::string logFileName;
    std::string logDir;
    size_t maxFileSize;
    int maxBackupFiles;
    LogLevel currentLevel;
    std::mutex logMutex;

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

    void rotateLogs() {
        if (!logFile.is_open()) return;

        logFile.seekp(0, std::ios::end);
        size_t currentSize = logFile.tellp();

        if (currentSize >= maxFileSize) {
            logFile.close();

            for (int i = maxBackupFiles - 1; i > 0; --i) {
                std::string oldName = logDir + "/" + logFileName + "." + std::to_string(i);
                std::string newName = logDir + "/" + logFileName + "." + std::to_string(i + 1);

                if (std::filesystem::exists(oldName)) {
                    std::filesystem::rename(oldName, newName);
                }
            }

            std::string firstBackup = logDir + "/" + logFileName + ".1";
            std::filesystem::rename(logDir + "/" + logFileName, firstBackup);

            openLogFile();
        }
    }

    void openLogFile() {
        std::filesystem::create_directories(logDir);
        logFile.open(logDir + "/" + logFileName, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file: " << logDir + "/" + logFileName << std::endl;
        }
    }

public:
    FileLogger(const std::string& directory = "logs", 
               const std::string& filename = "app.log",
               size_t maxSize = 1048576, 
               int backups = 5,
               LogLevel level = LogLevel::INFO)
        : logDir(directory), logFileName(filename), maxFileSize(maxSize), 
          maxBackupFiles(backups), currentLevel(level) {
        openLogFile();
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void setLogLevel(LogLevel level) {
        currentLevel = level;
    }

    void log(LogLevel level, const std::string& message) {
        if (level < currentLevel) return;

        std::lock_guard<std::mutex> lock(logMutex);
        
        rotateLogs();
        
        if (logFile.is_open()) {
            logFile << getCurrentTimestamp() << " [" << levelToString(level) << "] " 
                   << message << std::endl;
            logFile.flush();
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
    FileLogger logger("logs", "application.log", 1024 * 1024, 3, LogLevel::DEBUG);
    
    logger.debug("Starting application");
    logger.info("Configuration loaded successfully");
    logger.warning("Resource usage above threshold");
    logger.error("Failed to connect to database");
    
    for (int i = 0; i < 1000; ++i) {
        logger.info("Processing item " + std::to_string(i));
    }
}

int main() {
    exampleUsage();
    return 0;
}