
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
        : componentName_(componentName), minLevel_(minLevel) {}

    void setMinLevel(LogLevel level) {
        minLevel_ = level;
    }

    void setOutputStream(std::ostream& stream) {
        outputStream_ = &stream;
    }

    void log(LogLevel level, const std::string& message) {
        if (level < minLevel_) return;

        std::string levelStr;
        switch (level) {
            case LogLevel::DEBUG: levelStr = "DEBUG"; break;
            case LogLevel::INFO: levelStr = "INFO"; break;
            case LogLevel::WARNING: levelStr = "WARNING"; break;
            case LogLevel::ERROR: levelStr = "ERROR"; break;
        }

        std::time_t now = std::time(nullptr);
        char timeBuf[100];
        std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

        std::ostringstream logEntry;
        logEntry << "[" << timeBuf << "] "
                 << "[" << levelStr << "] "
                 << "[" << componentName_ << "] "
                 << message;

        if (outputStream_) {
            *outputStream_ << logEntry.str() << std::endl;
        } else {
            std::cout << logEntry.str() << std::endl;
        }
    }

    void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    void info(const std::string& message) { log(LogLevel::INFO, message); }
    void warning(const std::string& message) { log(LogLevel::WARNING, message); }
    void error(const std::string& message) { log(LogLevel::ERROR, message); }

private:
    std::string componentName_;
    LogLevel minLevel_;
    std::ostream* outputStream_ = nullptr;
};