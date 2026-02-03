
#include <iostream>
#include <fstream>
#include <string>
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
    std::ostream* outputStream;
    LogLevel currentLevel;
    bool ownsStream;

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
    Logger(LogLevel level = LogLevel::INFO) 
        : outputStream(&std::cout), currentLevel(level), ownsStream(false) {}

    Logger(const std::string& filename, LogLevel level = LogLevel::INFO)
        : currentLevel(level), ownsStream(true) {
        outputStream = new std::ofstream(filename, std::ios::app);
    }

    ~Logger() {
        if (ownsStream && outputStream) {
            delete outputStream;
        }
    }

    void setLevel(LogLevel level) {
        currentLevel = level;
    }

    void log(LogLevel level, const std::string& message) {
        if (level < currentLevel) return;

        *outputStream << "[" << getTimestamp() << "] "
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

void demonstrateLogger() {
    Logger consoleLogger(LogLevel::DEBUG);
    
    consoleLogger.debug("This is a debug message");
    consoleLogger.info("Application started");
    consoleLogger.warning("Low disk space detected");
    consoleLogger.error("Failed to connect to database");

    Logger fileLogger("app.log", LogLevel::INFO);
    fileLogger.info("Logging to file initialized");
    fileLogger.warning("Configuration file not found, using defaults");
}

int main() {
    demonstrateLogger();
    return 0;
}