
#include <fstream>
#include <string>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

class FileLogger {
public:
    explicit FileLogger(const std::string& baseName, size_t maxSize = 1048576, int maxFiles = 5)
        : baseFilename(baseName), maxFileSize(maxSize), maxBackupFiles(maxFiles), currentSize(0) {
        openLogFile();
    }

    ~FileLogger() {
        if (logStream.is_open()) {
            logStream.close();
        }
    }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        if (!logStream.is_open()) {
            return;
        }

        std::string entry = getCurrentTimestamp() + " - " + message + "\n";
        logStream << entry;
        logStream.flush();
        currentSize += entry.size();

        if (currentSize >= maxFileSize) {
            rotateLog();
        }
    }

private:
    std::string baseFilename;
    size_t maxFileSize;
    int maxBackupFiles;
    size_t currentSize;
    std::ofstream logStream;
    std::mutex logMutex;

    void openLogFile() {
        logStream.open(baseFilename, std::ios::app);
        if (logStream.is_open()) {
            currentSize = fs::file_size(baseFilename);
        }
    }

    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

    void rotateLog() {
        logStream.close();

        for (int i = maxBackupFiles - 1; i > 0; --i) {
            fs::path oldFile = baseFilename + "." + std::to_string(i);
            fs::path newFile = baseFilename + "." + std::to_string(i + 1);

            if (fs::exists(oldFile)) {
                if (fs::exists(newFile)) {
                    fs::remove(newFile);
                }
                fs::rename(oldFile, newFile);
            }
        }

        fs::path firstBackup = baseFilename + ".1";
        if (fs::exists(firstBackup)) {
            fs::remove(firstBackup);
        }
        fs::rename(baseFilename, firstBackup);

        openLogFile();
    }
};
#include <fstream>
#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

class FileLogger {
private:
    std::ofstream logFile;
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    std::string levelToString(LogLevel level) {
        switch(level) {
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

public:
    FileLogger(const std::string& filename) {
        logFile.open(filename, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file: " << filename << std::endl;
        }
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(LogLevel level, const std::string& message) {
        if (!logFile.is_open()) return;
        
        logFile << "[" << getCurrentTimestamp() << "] "
                << "[" << levelToString(level) << "] "
                << message << std::endl;
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

int main() {
    FileLogger logger("application.log");
    
    logger.info("Application started");
    logger.warning("Low disk space detected");
    logger.error("Failed to connect to database");
    
    return 0;
}