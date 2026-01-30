
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
    Logger(const std::string& filename = "", LogLevel minLevel = LogLevel::INFO)
        : minLevel_(minLevel), outputStream_(nullptr) {
        if (!filename.empty()) {
            fileStream_.open(filename, std::ios::app);
            if (fileStream_.is_open()) {
                outputStream_ = &fileStream_;
            } else {
                std::cerr << "Failed to open log file: " << filename << ". Defaulting to stdout." << std::endl;
                outputStream_ = &std::cout;
            }
        } else {
            outputStream_ = &std::cout;
        }
    }

    ~Logger() {
        if (fileStream_.is_open()) {
            fileStream_.close();
        }
    }

    void log(LogLevel level, const std::string& message) {
        if (level < minLevel_) return;

        std::ostringstream logEntry;
        logEntry << getCurrentTime() << " [" << levelToString(level) << "] " << message;

        std::lock_guard<std::mutex> lock(logMutex_);
        if (outputStream_) {
            *outputStream_ << logEntry.str() << std::endl;
        }
    }

    void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    void info(const std::string& message) { log(LogLevel::INFO, message); }
    void warning(const std::string& message) { log(LogLevel::WARNING, message); }
    void error(const std::string& message) { log(LogLevel::ERROR, message); }

    void setMinLevel(LogLevel level) { minLevel_ = level; }

private:
    std::string getCurrentTime() {
        std::time_t now = std::time(nullptr);
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        return std::string(buffer);
    }

    std::string levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG:   return "DEBUG";
            case LogLevel::INFO:    return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR:   return "ERROR";
            default:                return "UNKNOWN";
        }
    }

    LogLevel minLevel_;
    std::ofstream fileStream_;
    std::ostream* outputStream_;
    std::mutex logMutex_;
};