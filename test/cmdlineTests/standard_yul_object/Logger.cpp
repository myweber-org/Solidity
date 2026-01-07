
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <queue>
#include <mutex>
#include <atomic>

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
    std::mutex logMutex;
    std::atomic<LogLevel> currentLevel{LogLevel::INFO};
    size_t maxFileSize;
    int maxBackupFiles;
    
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
        logFile.close();
        
        for (int i = maxBackupFiles - 1; i > 0; --i) {
            fs::path oldFile = logFileName + "." + std::to_string(i);
            fs::path newFile = logFileName + "." + std::to_string(i + 1);
            
            if (fs::exists(oldFile)) {
                if (fs::exists(newFile)) {
                    fs::remove(newFile);
                }
                fs::rename(oldFile, newFile);
            }
        }
        
        fs::path firstBackup = logFileName + ".1";
        if (fs::exists(firstBackup)) {
            fs::remove(firstBackup);
        }
        fs::rename(logFileName, firstBackup);
        
        logFile.open(logFileName, std::ios::out | std::ios::app);
    }
    
public:
    Logger(const std::string& filename = "application.log", 
           size_t maxSize = 10485760, // 10MB
           int backups = 5)
        : logFileName(filename), maxFileSize(maxSize), maxBackupFiles(backups) {
        
        logFile.open(logFileName, std::ios::out | std::ios::app);
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
        currentLevel = level;
    }
    
    void log(LogLevel level, const std::string& message) {
        if (level < currentLevel) return;
        
        std::lock_guard<std::mutex> lock(logMutex);
        
        if (!logFile.is_open()) return;
        
        std::string logEntry = "[" + getTimestamp() + "] [" + 
                              levelToString(level) + "] " + message + "\n";
        
        logFile << logEntry;
        logFile.flush();
        
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
    
    void critical(const std::string& message) {
        log(LogLevel::CRITICAL, message);
    }
};

class AsyncLogger : public Logger {
private:
    std::queue<std::pair<LogLevel, std::string>> logQueue;
    std::mutex queueMutex;
    std::atomic<bool> running{true};
    std::thread workerThread;
    
    void processQueue() {
        while (running || !logQueue.empty()) {
            std::pair<LogLevel, std::string> entry;
            bool hasEntry = false;
            
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                if (!logQueue.empty()) {
                    entry = logQueue.front();
                    logQueue.pop();
                    hasEntry = true;
                }
            }
            
            if (hasEntry) {
                Logger::log(entry.first, entry.second);
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
    
public:
    AsyncLogger(const std::string& filename = "application.log",
                size_t maxSize = 10485760,
                int backups = 5)
        : Logger(filename, maxSize, backups) {
        workerThread = std::thread(&AsyncLogger::processQueue, this);
    }
    
    ~AsyncLogger() {
        running = false;
        if (workerThread.joinable()) {
            workerThread.join();
        }
    }
    
    void log(LogLevel level, const std::string& message) override {
        std::lock_guard<std::mutex> lock(queueMutex);
        logQueue.emplace(level, message);
    }
};