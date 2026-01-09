
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
            }
        }
        if (!outputStream_) {
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

        std::ostringstream oss;
        oss << "[" << timeStr << "] [" << levelStr << "] " << message;
        *outputStream_ << oss.str() << std::endl;
    }

    void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    void info(const std::string& message) { log(LogLevel::INFO, message); }
    void warning(const std::string& message) { log(LogLevel::WARNING, message); }
    void error(const std::string& message) { log(LogLevel::ERROR, message); }

private:
    LogLevel minLevel_;
    std::ofstream fileStream_;
    std::ostream* outputStream_;
};

void exampleUsage() {
    Logger consoleLogger;
    consoleLogger.info("Application started");
    consoleLogger.debug("Debug information");
    consoleLogger.warning("This is a warning");
    consoleLogger.error("An error occurred");

    Logger fileLogger("app.log", LogLevel::DEBUG);
    fileLogger.info("Log entry written to file");
    fileLogger.debug("Detailed debug info for file");
}