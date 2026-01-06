
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

class Logger {
private:
    std::ofstream logFile;
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
        return std::string(buffer);
    }
    
    std::string levelToString(LogLevel level) {
        switch(level) {
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

public:
    Logger(const std::string& filename) {
        logFile.open(filename, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file: " << filename << std::endl;
        }
    }
    
    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }
    
    void log(LogLevel level, const std::string& message) {
        if (!logFile.is_open()) return;
        
        std::string timestamp = getCurrentTimestamp();
        std::string levelStr = levelToString(level);
        
        logFile << "[" << timestamp << "] [" << levelStr << "] " << message << std::endl;
        
        if (level == LogLevel::ERROR) {
            std::cerr << "ERROR: " << message << std::endl;
        }
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
    Logger logger("application.log");
    
    logger.info("Application started");
    logger.warning("Low disk space detected");
    logger.error("Failed to connect to database");
    
    int value = 42;
    logger.info("Processing value: " + std::to_string(value));
}

int main() {
    exampleUsage();
    return 0;
}