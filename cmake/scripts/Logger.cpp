
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <ctime>
#include <memory>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    Logger(const std::string& componentName, LogLevel minLevel = LogLevel::INFO)
        : componentName_(componentName), minLevel_(minLevel) {
        outputStream_ = &std::cout;
    }

    void setMinLevel(LogLevel level) {
        minLevel_ = level;
    }

    void setOutputStream(std::ostream& stream) {
        outputStream_ = &stream;
    }

    void log(LogLevel level, const std::string& message) {
        if (level < minLevel_) {
            return;
        }

        std::string levelStr;
        switch (level) {
            case LogLevel::DEBUG: levelStr = "DEBUG"; break;
            case LogLevel::INFO: levelStr = "INFO"; break;
            case LogLevel::WARNING: levelStr = "WARNING"; break;
            case LogLevel::ERROR: levelStr = "ERROR"; break;
        }

        std::time_t now = std::time(nullptr);
        std::tm* localTime = std::localtime(&now);
        char timeBuffer[20];
        std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", localTime);

        std::ostringstream logEntry;
        logEntry << "[" << timeBuffer << "] "
                 << "[" << levelStr << "] "
                 << "[" << componentName_ << "] "
                 << message;

        std::lock_guard<std::mutex> lock(logMutex_);
        if (outputStream_) {
            *outputStream_ << logEntry.str() << std::endl;
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

private:
    std::string componentName_;
    LogLevel minLevel_;
    std::ostream* outputStream_;
    std::mutex logMutex_;
};

void exampleUsage() {
    Logger logger("NetworkModule", LogLevel::DEBUG);
    
    logger.debug("Initializing network connection");
    logger.info("Connected to server at 192.168.1.100");
    logger.warning("High latency detected: 150ms");
    logger.error("Connection timeout after 30 seconds");

    Logger fileLogger("FileSystem", LogLevel::WARNING);
    std::ofstream logFile("application.log");
    fileLogger.setOutputStream(logFile);
    fileLogger.warning("Disk space below 10%");
    fileLogger.error("Failed to write to /var/log/app.log");
}