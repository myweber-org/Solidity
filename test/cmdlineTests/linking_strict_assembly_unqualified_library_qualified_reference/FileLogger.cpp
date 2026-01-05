
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>
#include <mutex>
#include <memory>

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

    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
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
    FileLogger(const std::string& filename, LogLevel level = LogLevel::INFO) 
        : currentLevel(level) {
        logFile.open(filename, std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Cannot open log file: " + filename);
        }
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

        logFile << "[" << getCurrentTimestamp() << "] "
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

    void flush() {
        std::lock_guard<std::mutex> lock(logMutex);
        if (logFile.is_open()) {
            logFile.flush();
        }
    }
};

std::unique_ptr<FileLogger> createLogger(const std::string& filename, LogLevel level = LogLevel::INFO) {
    return std::make_unique<FileLogger>(filename, level);
}

void exampleUsage() {
    auto logger = createLogger("application.log", LogLevel::DEBUG);
    
    logger->info("Application started");
    logger->debug("Initializing components");
    logger->warning("Configuration file not found, using defaults");
    logger->error("Failed to connect to database");
    
    logger->setLogLevel(LogLevel::WARNING);
    logger->debug("This debug message will not be logged");
    logger->warning("This warning will be logged");
    
    logger->flush();
}

int main() {
    try {
        exampleUsage();
        std::cout << "Logging completed. Check application.log file." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}