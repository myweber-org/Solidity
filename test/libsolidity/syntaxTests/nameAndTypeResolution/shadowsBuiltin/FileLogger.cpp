#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
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
    LogLevel currentLevel;
    std::mutex logMutex;
    bool enabled;

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::ostringstream oss;
        oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return oss.str();
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

public:
    FileLogger(const std::string& filename, LogLevel level = LogLevel::INFO) 
        : currentLevel(level), enabled(true) {
        logFile.open(filename, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file: " << filename << std::endl;
            enabled = false;
        }
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(logMutex);
        currentLevel = level;
    }

    void enable() {
        std::lock_guard<std::mutex> lock(logMutex);
        enabled = true;
    }

    void disable() {
        std::lock_guard<std::mutex> lock(logMutex);
        enabled = false;
    }

    void log(LogLevel level, const std::string& message) {
        if (!enabled || level < currentLevel || !logFile.is_open()) {
            return;
        }

        std::lock_guard<std::mutex> lock(logMutex);
        logFile << "[" << getTimestamp() << "] "
                << "[" << levelToString(level) << "] "
                << message << std::endl;
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

int main() {
    FileLogger logger("application.log", LogLevel::DEBUG);
    
    logger.info("Application started");
    logger.debug("Debug information");
    logger.warning("This is a warning");
    logger.error("An error occurred");
    
    logger.setLogLevel(LogLevel::WARNING);
    logger.info("This info message won't be logged");
    logger.warning("This warning will be logged");
    
    return 0;
}