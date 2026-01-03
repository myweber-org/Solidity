
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
private:
    LogLevel currentLevel;
    std::ostream* outputStream;
    bool ownStream;

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

public:
    Logger(LogLevel level = LogLevel::INFO, std::ostream* stream = &std::cout)
        : currentLevel(level), outputStream(stream), ownStream(false) {}

    Logger(LogLevel level, const std::string& filename)
        : currentLevel(level), ownStream(true) {
        outputStream = new std::ofstream(filename, std::ios::app);
    }

    ~Logger() {
        if (ownStream && outputStream) {
            delete outputStream;
        }
    }

    void setLogLevel(LogLevel level) {
        currentLevel = level;
    }

    void log(LogLevel level, const std::string& message) {
        if (level < currentLevel) return;

        std::ostringstream logEntry;
        logEntry << "[" << getTimestamp() << "] "
                 << "[" << levelToString(level) << "] "
                 << message;

        if (outputStream) {
            *outputStream << logEntry.str() << std::endl;
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
};

void exampleUsage() {
    Logger consoleLogger(LogLevel::DEBUG);
    consoleLogger.info("Application started");
    consoleLogger.debug("Debug information");
    consoleLogger.warning("This is a warning");
    consoleLogger.error("An error occurred");

    Logger fileLogger(LogLevel::INFO, "app.log");
    fileLogger.info("Log entry written to file");
    fileLogger.debug("This debug message won't appear in file");
}