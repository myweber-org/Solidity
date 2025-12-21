#include <fstream>
#include <mutex>
#include <string>
#include <chrono>
#include <iomanop>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class FileLogger {
private:
    std::ofstream logFile;
    std::mutex logMutex;
    LogLevel currentLevel;

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&time);
        
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
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
        : currentLevel(level) {
        logFile.open(filename, std::ios::app);
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

    void log(LogLevel level, const std::string& message) {
        if (level < currentLevel) return;

        std::lock_guard<std::mutex> lock(logMutex);
        if (!logFile.is_open()) return;

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