
#include <iostream>
#include <chrono>
#include <iomanip>
#include <string>
#include <mutex>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

class Logger {
private:
    static std::mutex logMutex;

    static std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::ostringstream oss;
        oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return oss.str();
    }

    static std::string levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

public:
    static void log(LogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        std::cout << "[" << getCurrentTimestamp() << "] "
                  << "[" << levelToString(level) << "] "
                  << message << std::endl;
    }

    static void info(const std::string& message) {
        log(LogLevel::INFO, message);
    }

    static void warning(const std::string& message) {
        log(LogLevel::WARNING, message);
    }

    static void error(const std::string& message) {
        log(LogLevel::ERROR, message);
    }
};

std::mutex Logger::logMutex;