
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <mutex>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void setLogFile(const std::string& filename) {
        std::lock_guard<std::mutex> lock(mutex_);
        fileStream_.open(filename, std::ios::app);
    }

    void setMinLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(mutex_);
        minLevel_ = level;
    }

    void log(LogLevel level, const std::string& message) {
        if (level < minLevel_) return;

        std::lock_guard<std::mutex> lock(mutex_);
        std::string levelStr = getLevelString(level);
        std::string logEntry = "[" + levelStr + "] " + message;

        if (fileStream_.is_open()) {
            fileStream_ << logEntry << std::endl;
        }
        
        std::cout << logEntry << std::endl;
    }

private:
    Logger() : minLevel_(LogLevel::INFO) {}
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string getLevelString(LogLevel level) {
        switch(level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

    std::ofstream fileStream_;
    LogLevel minLevel_;
    std::mutex mutex_;
};

#define LOG_DEBUG(msg) Logger::getInstance().log(LogLevel::DEBUG, msg)
#define LOG_INFO(msg) Logger::getInstance().log(LogLevel::INFO, msg)
#define LOG_WARNING(msg) Logger::getInstance().log(LogLevel::WARNING, msg)
#define LOG_ERROR(msg) Logger::getInstance().log(LogLevel::ERROR, msg)