#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <filesystem>
#include <mutex>

namespace fs = std::filesystem;

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class FileLogger {
private:
    std::ofstream logFile;
    std::string baseFilename;
    size_t maxFileSize;
    int maxBackupFiles;
    std::mutex logMutex;
    LogLevel currentLevel;

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
        return std::string(buffer);
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

    void rotateIfNeeded() {
        if (!logFile.is_open()) return;

        logFile.seekp(0, std::ios::end);
        size_t currentSize = logFile.tellp();

        if (currentSize >= maxFileSize) {
            logFile.close();

            for (int i = maxBackupFiles - 1; i > 0; --i) {
                fs::path oldFile = baseFilename + "." + std::to_string(i);
                fs::path newFile = baseFilename + "." + std::to_string(i + 1);

                if (fs::exists(oldFile)) {
                    fs::rename(oldFile, newFile);
                }
            }

            fs::path firstBackup = baseFilename + ".1";
            if (fs::exists(baseFilename)) {
                fs::rename(baseFilename, firstBackup);
            }

            logFile.open(baseFilename, std::ios::app);
        }
    }

public:
    FileLogger(const std::string& filename, size_t maxSize = 1048576, int backups = 5, LogLevel level = LogLevel::INFO)
        : baseFilename(filename), maxFileSize(maxSize), maxBackupFiles(backups), currentLevel(level) {
        logFile.open(baseFilename, std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Cannot open log file: " + filename);
        }
    }

    ~FileLogger() {
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
        
        rotateIfNeeded();
        
        if (logFile.is_open()) {
            logFile << "[" << getTimestamp() << "] "
                    << "[" << levelToString(level) << "] "
                    << message << std::endl;
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

    void flush() {
        std::lock_guard<std::mutex> lock(logMutex);
        if (logFile.is_open()) {
            logFile.flush();
        }
    }
};

int main() {
    try {
        FileLogger logger("application.log", 1024, 3, LogLevel::DEBUG);
        
        logger.debug("Starting application");
        logger.info("Initialization complete");
        logger.warning("Resource usage above threshold");
        logger.error("Failed to connect to database");
        
        for (int i = 0; i < 100; ++i) {
            logger.info("Log entry number: " + std::to_string(i));
        }
        
        logger.flush();
        std::cout << "Logging completed. Check application.log file." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Logger error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}