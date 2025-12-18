
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <mutex>
#include <ctime>

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

    void setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(mutex_);
        currentLevel_ = level;
    }

    void setOutputStream(std::ostream* stream) {
        std::lock_guard<std::mutex> lock(mutex_);
        outputStream_ = stream;
    }

    void log(LogLevel level, const std::string& message) {
        if (level < currentLevel_) return;

        std::lock_guard<std::mutex> lock(mutex_);
        if (!outputStream_) return;

        *outputStream_ << getTimestamp() << " [" << levelToString(level) << "] " << message << std::endl;
    }

    void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    void info(const std::string& message) { log(LogLevel::INFO, message); }
    void warning(const std::string& message) { log(LogLevel::WARNING, message); }
    void error(const std::string& message) { log(LogLevel::ERROR, message); }

private:
    Logger() : currentLevel_(LogLevel::INFO), outputStream_(&std::cout) {}
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string getTimestamp() {
        std::time_t now = std::time(nullptr);
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
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

    LogLevel currentLevel_;
    std::ostream* outputStream_;
    std::mutex mutex_;
};

#define LOG_DEBUG(msg) Logger::getInstance().debug(msg)
#define LOG_INFO(msg) Logger::getInstance().info(msg)
#define LOG_WARNING(msg) Logger::getInstance().warning(msg)
#define LOG_ERROR(msg) Logger::getInstance().error(msg)