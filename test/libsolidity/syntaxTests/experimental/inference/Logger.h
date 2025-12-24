
#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>
#include <sstream>
#include <mutex>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
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

    void log(LogLevel level, const std::string& message) {
        if (level < currentLevel_) return;

        std::lock_guard<std::mutex> lock(mutex_);
        std::string levelStr = levelToString(level);
        std::cout << "[" << levelStr << "] " << message << std::endl;
    }

    template<typename... Args>
    void log(LogLevel level, Args... args) {
        if (level < currentLevel_) return;

        std::lock_guard<std::mutex> lock(mutex_);
        std::ostringstream stream;
        (stream << ... << args);
        std::string levelStr = levelToString(level);
        std::cout << "[" << levelStr << "] " << stream.str() << std::endl;
    }

private:
    Logger() : currentLevel_(LogLevel::INFO) {}
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

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

    LogLevel currentLevel_;
    std::mutex mutex_;
};

#define LOG_DEBUG(...) Logger::getInstance().log(LogLevel::DEBUG, __VA_ARGS__)
#define LOG_INFO(...) Logger::getInstance().log(LogLevel::INFO, __VA_ARGS__)
#define LOG_WARNING(...) Logger::getInstance().log(LogLevel::WARNING, __VA_ARGS__)
#define LOG_ERROR(...) Logger::getInstance().log(LogLevel::ERROR, __VA_ARGS__)
#define LOG_CRITICAL(...) Logger::getInstance().log(LogLevel::CRITICAL, __VA_ARGS__)

#endif