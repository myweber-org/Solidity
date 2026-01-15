#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

class FileLogger {
public:
    static FileLogger& getInstance(const std::string& basePath = "logs", size_t maxFileSize = 1048576, int maxBackupFiles = 5) {
        static FileLogger instance(basePath, maxFileSize, maxBackupFiles);
        return instance;
    }

    void log(LogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (level < currentLevel_) return;
        
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        
        std::string levelStr;
        switch(level) {
            case LogLevel::DEBUG: levelStr = "DEBUG"; break;
            case LogLevel::INFO: levelStr = "INFO"; break;
            case LogLevel::WARNING: levelStr = "WARN"; break;
            case LogLevel::ERROR: levelStr = "ERROR"; break;
            case LogLevel::CRITICAL: levelStr = "CRITICAL"; break;
        }
        
        std::string logEntry = "[" + ss.str() + "] [" + levelStr + "] " + message + "\n";
        
        checkRotation();
        if (logFile_.is_open()) {
            logFile_ << logEntry;
            logFile_.flush();
            currentSize_ += logEntry.size();
        }
    }
    
    void setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(mutex_);
        currentLevel_ = level;
    }
    
    FileLogger(const FileLogger&) = delete;
    FileLogger& operator=(const FileLogger&) = delete;

private:
    FileLogger(const std::string& basePath, size_t maxFileSize, int maxBackupFiles)
        : basePath_(basePath), maxFileSize_(maxFileSize), maxBackupFiles_(maxBackupFiles), currentSize_(0), currentLevel_(LogLevel::INFO) {
        
        std::filesystem::create_directories(basePath_);
        openNewLogFile();
    }
    
    void openNewLogFile() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream filename;
        filename << basePath_ << "/app_";
        filename << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
        filename << ".log";
        
        currentFilename_ = filename.str();
        logFile_.open(currentFilename_, std::ios::app);
        currentSize_ = std::filesystem::exists(currentFilename_) ? std::filesystem::file_size(currentFilename_) : 0;
    }
    
    void checkRotation() {
        if (currentSize_ >= maxFileSize_ || !logFile_.is_open()) {
            logFile_.close();
            rotateLogFiles();
            openNewLogFile();
        }
    }
    
    void rotateLogFiles() {
        try {
            std::vector<std::filesystem::path> logFiles;
            for (const auto& entry : std::filesystem::directory_iterator(basePath_)) {
                if (entry.is_regular_file() && entry.path().extension() == ".log") {
                    logFiles.push_back(entry.path());
                }
            }
            
            std::sort(logFiles.begin(), logFiles.end(), 
                     [](const std::filesystem::path& a, const std::filesystem::path& b) {
                         return std::filesystem::last_write_time(a) < std::filesystem::last_write_time(b);
                     });
            
            while (logFiles.size() >= static_cast<size_t>(maxBackupFiles_)) {
                std::filesystem::remove(logFiles.front());
                logFiles.erase(logFiles.begin());
            }
        } catch (...) {
            // Silently handle filesystem errors
        }
    }
    
    std::ofstream logFile_;
    std::mutex mutex_;
    std::string basePath_;
    std::string currentFilename_;
    size_t maxFileSize_;
    int maxBackupFiles_;
    size_t currentSize_;
    LogLevel currentLevel_;
};

// Convenience macros for logging
#define LOG_DEBUG(msg) FileLogger::getInstance().log(LogLevel::DEBUG, msg)
#define LOG_INFO(msg) FileLogger::getInstance().log(LogLevel::INFO, msg)
#define LOG_WARNING(msg) FileLogger::getInstance().log(LogLevel::WARNING, msg)
#define LOG_ERROR(msg) FileLogger::getInstance().log(LogLevel::ERROR, msg)
#define LOG_CRITICAL(msg) FileLogger::getInstance().log(LogLevel::CRITICAL, msg)