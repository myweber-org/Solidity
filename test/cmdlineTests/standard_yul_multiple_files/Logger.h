#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>

class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void initialize(const std::string& filename, Level minLevel = Level::INFO) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (logFile_.is_open()) {
            logFile_.close();
        }
        logFile_.open(filename, std::ios::app);
        minLevel_ = minLevel;
    }

    void log(Level level, const std::string& message) {
        if (level < minLevel_) return;

        std::lock_guard<std::mutex> lock(mutex_);
        if (!logFile_.is_open()) return;

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        logFile_ << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] ";
        
        switch (level) {
            case Level::DEBUG: logFile_ << "[DEBUG] "; break;
            case Level::INFO: logFile_ << "[INFO] "; break;
            case Level::WARNING: logFile_ << "[WARNING] "; break;
            case Level::ERROR: logFile_ << "[ERROR] "; break;
        }
        
        logFile_ << message << std::endl;
    }

    void debug(const std::string& message) { log(Level::DEBUG, message); }
    void info(const std::string& message) { log(Level::INFO, message); }
    void warning(const std::string& message) { log(Level::WARNING, message); }
    void error(const std::string& message) { log(Level::ERROR, message); }

    ~Logger() {
        if (logFile_.is_open()) {
            logFile_.close();
        }
    }

private:
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::ofstream logFile_;
    std::mutex mutex_;
    Level minLevel_ = Level::INFO;
};

#endif