
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>

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
    std::string logDir;
    size_t maxFileSize;
    int maxBackupFiles;
    LogLevel currentLevel;
    std::mutex logMutex;
    std::queue<std::string> logQueue;
    std::condition_variable queueCondition;
    std::thread writerThread;
    bool stopThread;

    void rotateLogs() {
        if (fs::exists(logFileName)) {
            auto fileSize = fs::file_size(logFileName);
            if (fileSize >= maxFileSize) {
                for (int i = maxBackupFiles - 1; i > 0; --i) {
                    std::string oldName = logFileName + "." + std::to_string(i);
                    std::string newName = logFileName + "." + std::to_string(i + 1);
                    if (fs::exists(oldName)) {
                        fs::rename(oldName, newName);
                    }
                }
                std::string firstBackup = logFileName + ".1";
                fs::rename(logFileName, firstBackup);
            }
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

    std::string levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::CRITICAL: return "CRITICAL";
            default: return "UNKNOWN";
        }
    }

    void writerFunction() {
        while (true) {
            std::string logEntry;
            {
                std::unique_lock<std::mutex> lock(logMutex);
                queueCondition.wait(lock, [this]() {
                    return !logQueue.empty() || stopThread;
                });

                if (stopThread && logQueue.empty()) {
                    break;
                }

                if (!logQueue.empty()) {
                    logEntry = logQueue.front();
                    logQueue.pop();
                }
            }

            if (!logEntry.empty()) {
                rotateLogs();
                logFile.open(logFileName, std::ios::app);
                if (logFile.is_open()) {
                    logFile << logEntry << std::endl;
                    logFile.close();
                }
            }
        }
    }

public:
    Logger(const std::string& directory = "logs", 
           const std::string& baseName = "app",
           size_t maxSize = 1048576,
           int maxBackups = 5,
           LogLevel level = LogLevel::INFO)
        : maxFileSize(maxSize), maxBackupFiles(maxBackups), 
          currentLevel(level), stopThread(false) {
        
        logDir = directory;
        if (!fs::exists(logDir)) {
            fs::create_directories(logDir);
        }
        
        logFileName = logDir + "/" + baseName + ".log";
        writerThread = std::thread(&Logger::writerFunction, this);
    }

    ~Logger() {
        {
            std::lock_guard<std::mutex> lock(logMutex);
            stopThread = true;
        }
        queueCondition.notify_one();
        writerThread.join();
    }

    void log(LogLevel level, const std::string& message) {
        if (level < currentLevel) return;

        std::stringstream ss;
        ss << "[" << getCurrentTimestamp() << "] "
           << "[" << levelToString(level) << "] "
           << message;

        {
            std::lock_guard<std::mutex> lock(logMutex);
            logQueue.push(ss.str());
        }
        queueCondition.notify_one();
    }

    void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    void info(const std::string& message) { log(LogLevel::INFO, message); }
    void warning(const std::string& message) { log(LogLevel::WARNING, message); }
    void error(const std::string& message) { log(LogLevel::ERROR, message); }
    void critical(const std::string& message) { log(LogLevel::CRITICAL, message); }

    void setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(logMutex);
        currentLevel = level;
    }
};