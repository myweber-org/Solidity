
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
    Logger(const std::string& componentName) : component(componentName) {}

    void setLogLevel(LogLevel level) {
        currentLevel = level;
    }

    void setOutputStream(std::ostream* stream) {
        outputStream = stream;
        ownStream = false;
    }

    void setLogFile(const std::string& filename) {
        fileStream = std::make_unique<std::ofstream>(filename, std::ios::app);
        outputStream = fileStream.get();
        ownStream = true;
    }

    void log(LogLevel level, const std::string& message) {
        if (level < currentLevel) return;

        std::string levelStr;
        switch (level) {
            case LogLevel::DEBUG: levelStr = "DEBUG"; break;
            case LogLevel::INFO: levelStr = "INFO"; break;
            case LogLevel::WARNING: levelStr = "WARNING"; break;
            case LogLevel::ERROR: levelStr = "ERROR"; break;
        }

        std::time_t now = std::time(nullptr);
        char timeStr[100];
        std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

        std::ostringstream logEntry;
        logEntry << "[" << timeStr << "] [" << levelStr << "] [" << component << "] " << message;

        if (outputStream) {
            *outputStream << logEntry.str() << std::endl;
        }
    }

    void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    void info(const std::string& message) { log(LogLevel::INFO, message); }
    void warning(const std::string& message) { log(LogLevel::WARNING, message); }
    void error(const std::string& message) { log(LogLevel::ERROR, message); }

    ~Logger() {
        if (ownStream && fileStream) {
            fileStream->close();
        }
    }

private:
    std::string component;
    LogLevel currentLevel = LogLevel::INFO;
    std::ostream* outputStream = &std::cout;
    std::unique_ptr<std::ofstream> fileStream;
    bool ownStream = false;
};