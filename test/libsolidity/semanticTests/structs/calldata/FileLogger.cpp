
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>

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
    size_t maxFiles;
    size_t currentFileIndex;
    size_t currentFileSize;

    std::string getTimestamp() {
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

    void rotateIfNeeded() {
        if (currentFileSize >= maxFileSize) {
            logFile.close();
            currentFileIndex = (currentFileIndex + 1) % maxFiles;
            
            std::string newFilename = baseFilename + "_" + std::to_string(currentFileIndex) + ".log";
            std::filesystem::remove(newFilename);
            
            logFile.open(newFilename, std::ios::out | std::ios::app);
            currentFileSize = 0;
        }
    }

public:
    FileLogger(const std::string& filename, size_t maxSize = 1048576, size_t maxFileCount = 5)
        : baseFilename(filename), maxFileSize(maxSize), maxFiles(maxFileCount), 
          currentFileIndex(0), currentFileSize(0) {
        
        std::string firstFilename = baseFilename + "_0.log";
        logFile.open(firstFilename, std::ios::out | std::ios::app);
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(LogLevel level, const std::string& message) {
        if (!logFile.is_open()) return;

        std::string logEntry = "[" + getTimestamp() + "] [" + levelToString(level) + "] " + message + "\n";
        
        logFile << logEntry;
        logFile.flush();
        
        currentFileSize += logEntry.size();
        rotateIfNeeded();
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