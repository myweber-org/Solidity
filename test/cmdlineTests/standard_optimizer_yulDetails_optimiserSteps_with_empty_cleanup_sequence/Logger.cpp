
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
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

    void setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(mutex_);
        currentLevel_ = level;
    }

    void setOutputStream(std::ostream& stream) {
        std::lock_guard<std::mutex> lock(mutex_);
        outputStream_ = &stream;
    }

    void setLogFile(const std::string& filename) {
        std::lock_guard<std::mutex> lock(mutex_);
        fileStream_.open(filename, std::ios::app);
        if (fileStream_.is_open()) {
            fileOutputEnabled_ = true;
        }
    }

    void log(LogLevel level, const std::string& message) {
        if (level < currentLevel_) return;

        std::lock_guard<std::mutex> lock(mutex_);
        std::string levelStr = getLevelString(level);
        std::string formattedMessage = formatLogMessage(levelStr, message);

        if (outputStream_) {
            *outputStream_ << formattedMessage << std::endl;
        }

        if (fileOutputEnabled_ && fileStream_.is_open()) {
            fileStream_ << formattedMessage << std::endl;
        }
    }

    void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    void info(const std::string& message) { log(LogLevel::INFO, message); }
    void warning(const std::string& message) { log(LogLevel::WARNING, message); }
    void error(const std::string& message) { log(LogLevel::ERROR, message); }

private:
    Logger() : currentLevel_(LogLevel::INFO), outputStream_(&std::cout), fileOutputEnabled_(false) {}

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string getLevelString(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

    std::string formatLogMessage(const std::string& level, const std::string& message) {
        std::time_t now = std::time(nullptr);
        std::tm* localTime = std::localtime(&now);
        
        std::ostringstream oss;
        oss << "[" << (localTime->tm_year + 1900) << "-"
            << (localTime->tm_mon + 1) << "-"
            << localTime->tm_mday << " "
            << localTime->tm_hour << ":"
            << localTime->tm_min << ":"
            << localTime->tm_sec << "] "
            << "[" << level << "] "
            << message;
        
        return oss.str();
    }

    LogLevel currentLevel_;
    std::ostream* outputStream_;
    std::ofstream fileStream_;
    bool fileOutputEnabled_;
    std::mutex mutex_;
};

#define LOG_DEBUG(msg) Logger::getInstance().debug(msg)
#define LOG_INFO(msg) Logger::getInstance().info(msg)
#define LOG_WARNING(msg) Logger::getInstance().warning(msg)
#define LOG_ERROR(msg) Logger::getInstance().error(msg)