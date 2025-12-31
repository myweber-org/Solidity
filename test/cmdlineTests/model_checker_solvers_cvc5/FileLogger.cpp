#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <filesystem>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

class FileLogger {
private:
    std::ofstream logFile;
    std::string baseFilename;
    std::string currentFilename;
    size_t maxFileSize;
    int maxBackupFiles;
    LogLevel currentLevel;
    std::mutex writeMutex;
    
    std::queue<std::string> logQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    std::thread writerThread;
    bool stopThread;
    
    void rotateLogFile() {
        if (logFile.is_open()) {
            logFile.close();
        }
        
        for (int i = maxBackupFiles - 1; i > 0; --i) {
            std::string oldName = baseFilename + "." + std::to_string(i);
            std::string newName = baseFilename + "." + std::to_string(i + 1);
            
            if (std::filesystem::exists(oldName)) {
                std::filesystem::rename(oldName, newName);
            }
        }
        
        std::string firstBackup = baseFilename + ".1";
        if (std::filesystem::exists(currentFilename)) {
            std::filesystem::rename(currentFilename, firstBackup);
        }
        
        logFile.open(currentFilename, std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Cannot open log file: " + currentFilename);
        }
    }
    
    void writerFunction() {
        while (true) {
            std::string message;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                queueCondition.wait(lock, [this]() {
                    return !logQueue.empty() || stopThread;
                });
                
                if (stopThread && logQueue.empty()) {
                    return;
                }
                
                message = logQueue.front();
                logQueue.pop();
            }
            
            {
                std::lock_guard<std::mutex> lock(writeMutex);
                logFile << message << std::endl;
                logFile.flush();
                
                if (logFile.tellp() > maxFileSize) {
                    rotateLogFile();
                }
            }
        }
    }
    
    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::string timestamp(30, '\0');
        std::strftime(&timestamp[0], timestamp.size(), 
                     "%Y-%m-%d %H:%M:%S", std::localtime(&time));
        timestamp.resize(19);
        timestamp += "." + std::to_string(ms.count());
        
        return timestamp;
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

public:
    FileLogger(const std::string& filename, 
               size_t maxSize = 10 * 1024 * 1024,  // 10 MB
               int backups = 5,
               LogLevel level = LogLevel::INFO)
        : baseFilename(filename), 
          currentFilename(filename),
          maxFileSize(maxSize),
          maxBackupFiles(backups),
          currentLevel(level),
          stopThread(false) {
        
        logFile.open(currentFilename, std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Cannot open log file: " + currentFilename);
        }
        
        writerThread = std::thread(&FileLogger::writerFunction, this);
    }
    
    ~FileLogger() {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            stopThread = true;
        }
        queueCondition.notify_one();
        writerThread.join();
        
        if (logFile.is_open()) {
            logFile.close();
        }
    }
    
    void setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(writeMutex);
        currentLevel = level;
    }
    
    void log(LogLevel level, const std::string& message) {
        if (level < currentLevel) {
            return;
        }
        
        std::string logEntry = getTimestamp() + " [" + 
                              levelToString(level) + "] " + message;
        
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            logQueue.push(logEntry);
        }
        queueCondition.notify_one();
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
    
    void flush() {
        std::lock_guard<std::mutex> lock(writeMutex);
        if (logFile.is_open()) {
            logFile.flush();
        }
    }
};

void exampleUsage() {
    FileLogger logger("application.log", 5 * 1024 * 1024, 3, LogLevel::DEBUG);
    
    logger.debug("Debug message for detailed information");
    logger.info("Application started successfully");
    logger.warning("Resource usage is above 80%");
    logger.error("Failed to connect to database");
    logger.critical("System shutting down due to critical error");
    
    for (int i = 0; i < 100; ++i) {
        logger.info("Processing item " + std::to_string(i));
    }
    
    logger.flush();
}

int main() {
    try {
        exampleUsage();
        std::cout << "Logging example completed. Check application.log file." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}