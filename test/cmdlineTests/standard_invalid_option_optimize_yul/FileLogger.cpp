
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
    std::string filePath;
    std::mutex logMutex;
    size_t maxFileSize;
    int maxBackupFiles;

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

    void rotateIfNeeded() {
        if (!std::filesystem::exists(filePath)) return;
        
        auto fileSize = std::filesystem::file_size(filePath);
        if (fileSize < maxFileSize) return;

        logFile.close();

        for (int i = maxBackupFiles - 1; i > 0; --i) {
            std::string oldName = filePath + "." + std::to_string(i);
            std::string newName = filePath + "." + std::to_string(i + 1);
            
            if (std::filesystem::exists(oldName)) {
                if (std::filesystem::exists(newName)) {
                    std::filesystem::remove(newName);
                }
                std::filesystem::rename(oldName, newName);
            }
        }

        std::string firstBackup = filePath + ".1";
        if (std::filesystem::exists(firstBackup)) {
            std::filesystem::remove(firstBackup);
        }
        std::filesystem::rename(filePath, firstBackup);

        logFile.open(filePath, std::ios::app);
    }

public:
    FileLogger(const std::string& path, size_t maxSize = 1048576, int maxBackups = 5) 
        : filePath(path), maxFileSize(maxSize), maxBackupFiles(maxBackups) {
        
        std::filesystem::create_directories(std::filesystem::path(path).parent_path());
        logFile.open(path, std::ios::app);
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(LogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        
        rotateIfNeeded();
        
        if (logFile.is_open()) {
            logFile << "[" << getCurrentTimestamp() << "] "
                    << "[" << levelToString(level) << "] "
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