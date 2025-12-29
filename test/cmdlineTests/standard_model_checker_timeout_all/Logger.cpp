
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

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
    bool isFileOwned;

    std::string getCurrentTime() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

    std::string levelToString(LogLevel level) {
        switch(level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARN";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

public:
    Logger() : outputStream(&std::cout), currentLevel(LogLevel::INFO), isFileOwned(false) {}

    explicit Logger(const std::string& filename) : currentLevel(LogLevel::INFO), isFileOwned(true) {
        outputStream = new std::ofstream(filename, std::ios::app);
    }

    explicit Logger(std::ostream& stream) : outputStream(&stream), currentLevel(LogLevel::INFO), isFileOwned(false) {}

    ~Logger() {
        if (isFileOwned && outputStream) {
            delete outputStream;
        }
    }

    void setLogLevel(LogLevel level) {
        currentLevel = level;
    }

    template<typename T>
    void log(LogLevel level, const T& message) {
        if (level >= currentLevel) {
            *outputStream << "[" << getCurrentTime() << "] "
                         << "[" << levelToString(level) << "] "
                         << message << std::endl;
        }
    }

    template<typename T>
    void debug(const T& message) {
        log(LogLevel::DEBUG, message);
    }

    template<typename T>
    void info(const T& message) {
        log(LogLevel::INFO, message);
    }

    template<typename T>
    void warning(const T& message) {
        log(LogLevel::WARNING, message);
    }

    template<typename T>
    void error(const T& message) {
        log(LogLevel::ERROR, message);
    }
};