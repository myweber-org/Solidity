
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

    void setOutputStream(std::ostream* stream) {
        outputStream_ = stream;
    }

    void setLogFile(const std::string& filename) {
        logFile_.open(filename, std::ios::app);
        if (!logFile_.is_open()) {
            std::cerr << "Failed to open log file: " << filename << std::endl;
        }
    }

    void log(LogLevel level, const std::string& message) {
        if (level < minLevel_) return;

        std::ostringstream logEntry;
        logEntry << getCurrentTime() << " [" << levelToString(level) << "] "
                 << "[" << componentName_ << "] " << message;

        std::string finalMessage = logEntry.str();

        if (outputStream_) {
            *outputStream_ << finalMessage << std::endl;
        }

        if (logFile_.is_open()) {
            logFile_ << finalMessage << std::endl;
        }
    }

    void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    void info(const std::string& message) { log(LogLevel::INFO, message); }
    void warning(const std::string& message) { log(LogLevel::WARNING, message); }
    void error(const std::string& message) { log(LogLevel::ERROR, message); }

    void setMinLogLevel(LogLevel level) {
        minLevel_ = level;
    }

    ~Logger() {
        if (logFile_.is_open()) {
            logFile_.close();
        }
    }

private:
    std::string getCurrentTime() {
        std::time_t now = std::time(nullptr);
        char timeStr[100];
        std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        return std::string(timeStr);
    }

    std::string levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

    std::string componentName_;
    LogLevel minLevel_;
    std::ostream* outputStream_ = &std::cout;
    std::ofstream logFile_;
};

void exampleUsage() {
    Logger appLogger("Application", LogLevel::DEBUG);
    appLogger.setLogFile("application.log");

    appLogger.debug("Starting application initialization");
    appLogger.info("Application started successfully");
    appLogger.warning("Resource usage is above 80%");
    appLogger.error("Failed to connect to database");

    Logger networkLogger("Network");
    networkLogger.setMinLogLevel(LogLevel::WARNING);
    networkLogger.warning("High latency detected");
    networkLogger.info("This message won't be logged due to min level");
}

int main() {
    exampleUsage();
    return 0;
}