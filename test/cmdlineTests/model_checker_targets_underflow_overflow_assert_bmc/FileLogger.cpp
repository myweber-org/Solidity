#include <fstream>
#include <string>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class FileLogger {
public:
    explicit FileLogger(const std::string& filename) : logFile(filename, std::ios::app) {
        if (!logFile.is_open()) {
            throw std::runtime_error("Failed to open log file");
        }
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(LogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        if (!logFile.is_open()) return;

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        logFile << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        logFile << "." << std::setfill('0') << std::setw(3) << ms.count() << "] ";
        
        switch (level) {
            case LogLevel::DEBUG: logFile << "[DEBUG] "; break;
            case LogLevel::INFO: logFile << "[INFO] "; break;
            case LogLevel::WARNING: logFile << "[WARNING] "; break;
            case LogLevel::ERROR: logFile << "[ERROR] "; break;
        }
        
        logFile << message << std::endl;
        logFile.flush();
    }

    void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    void info(const std::string& message) { log(LogLevel::INFO, message); }
    void warning(const std::string& message) { log(LogLevel::WARNING, message); }
    void error(const std::string& message) { log(LogLevel::ERROR, message); }

private:
    std::ofstream logFile;
    std::mutex logMutex;
};
#include <fstream>
#include <string>
#include <filesystem>
#include <chrono>
#include <iomanip>

namespace fs = std::filesystem;

class FileLogger {
private:
    std::ofstream logFile;
    std::string baseName;
    size_t maxSize;
    int maxFiles;
    size_t currentSize;

    void rotateIfNeeded() {
        if (currentSize >= maxSize) {
            logFile.close();
            
            for (int i = maxFiles - 1; i > 0; --i) {
                fs::path oldFile = baseName + "." + std::to_string(i);
                fs::path newFile = baseName + "." + std::to_string(i + 1);
                
                if (fs::exists(oldFile)) {
                    fs::rename(oldFile, newFile);
                }
            }
            
            fs::path firstBackup = baseName + ".1";
            fs::rename(baseName, firstBackup);
            
            openLogFile();
        }
    }

    void openLogFile() {
        logFile.open(baseName, std::ios::app);
        currentSize = fs::file_size(baseName);
    }

public:
    FileLogger(const std::string& filename, size_t maxFileSize = 1048576, int backupCount = 5)
        : baseName(filename), maxSize(maxFileSize), maxFiles(backupCount), currentSize(0) {
        
        if (!fs::exists(baseName)) {
            std::ofstream initFile(baseName);
            initFile.close();
        }
        
        openLogFile();
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        logFile << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] ";
        logFile << message << std::endl;
        
        currentSize = logFile.tellp();
        rotateIfNeeded();
    }

    void logError(const std::string& errorMessage) {
        log("[ERROR] " + errorMessage);
    }

    void logWarning(const std::string& warningMessage) {
        log("[WARNING] " + warningMessage);
    }

    void logInfo(const std::string& infoMessage) {
        log("[INFO] " + infoMessage);
    }
};