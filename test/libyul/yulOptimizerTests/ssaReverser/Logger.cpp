
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <chrono>
#include <iomanip>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
private:
    std::ofstream logFile;
    LogLevel currentLevel;
    std::mutex logMutex;
    bool outputToConsole;
    
    std::string getLevelString(LogLevel level) {
        switch(level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }
    
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }
    
public:
    Logger(const std::string& filename = "application.log", 
           LogLevel level = LogLevel::INFO,
           bool consoleOutput = true)
        : currentLevel(level), outputToConsole(consoleOutput) {
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
    
    void setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(logMutex);
        currentLevel = level;
    }
    
    void log(LogLevel level, const std::string& message) {
        if (level < currentLevel) return;
        
        std::lock_guard<std::mutex> lock(logMutex);
        std::string logEntry = "[" + getCurrentTimestamp() + "] [" + 
                              getLevelString(level) + "] " + message;
        
        if (outputToConsole) {
            std::cout << logEntry << std::endl;
        }
        
        if (logFile.is_open()) {
            logFile << logEntry << std::endl;
            logFile.flush();
        }
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

void exampleUsage() {
    Logger logger("myapp.log", LogLevel::DEBUG, true);
    
    logger.debug("Application started");
    logger.info("Loading configuration");
    logger.warning("Deprecated API called");
    logger.error("Failed to connect to database");
    
    logger.setLogLevel(LogLevel::WARNING);
    logger.debug("This debug message won't appear");
    logger.info("This info message won't appear");
    logger.warning("This warning will appear");
}