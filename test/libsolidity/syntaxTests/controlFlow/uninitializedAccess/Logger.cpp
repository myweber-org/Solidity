
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
        : component(componentName), minimumLevel(minLevel) {}

    void setMinimumLevel(LogLevel level) {
        minimumLevel = level;
    }

    void addOutputStream(std::ostream& stream) {
        outputStreams.push_back(&stream);
    }

    void log(LogLevel level, const std::string& message) {
        if (level < minimumLevel) return;

        std::string levelStr;
        switch(level) {
            case LogLevel::DEBUG: levelStr = "DEBUG"; break;
            case LogLevel::INFO: levelStr = "INFO"; break;
            case LogLevel::WARNING: levelStr = "WARNING"; break;
            case LogLevel::ERROR: levelStr = "ERROR"; break;
        }

        std::time_t now = std::time(nullptr);
        char timeBuf[100];
        std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

        std::ostringstream formattedMessage;
        formattedMessage << "[" << timeBuf << "] "
                         << "[" << levelStr << "] "
                         << "[" << component << "] "
                         << message;

        std::string finalMessage = formattedMessage.str();

        for (auto stream : outputStreams) {
            if (stream) {
                *stream << finalMessage << std::endl;
            }
        }
    }

    void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    void info(const std::string& message) { log(LogLevel::INFO, message); }
    void warning(const std::string& message) { log(LogLevel::WARNING, message); }
    void error(const std::string& message) { log(LogLevel::ERROR, message); }

private:
    std::string component;
    LogLevel minimumLevel;
    std::vector<std::ostream*> outputStreams;
};

class FileLogger : public Logger {
public:
    FileLogger(const std::string& componentName, const std::string& filename, LogLevel minLevel = LogLevel::INFO)
        : Logger(componentName, minLevel), fileStream(filename, std::ios::app) {
        if (fileStream.is_open()) {
            addOutputStream(fileStream);
        }
    }

    ~FileLogger() {
        if (fileStream.is_open()) {
            fileStream.close();
        }
    }

private:
    std::ofstream fileStream;
};