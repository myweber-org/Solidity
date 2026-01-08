
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

enum LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
private:
    std::ofstream logFile;
    LogLevel currentLevel;
    std::string levelToString(LogLevel level) {
        switch(level) {
            case DEBUG: return "DEBUG";
            case INFO: return "INFO";
            case WARNING: return "WARNING";
            case ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }
    std::string getCurrentTime() {
        time_t now = time(0);
        char* dt = ctime(&now);
        std::string timeStr(dt);
        timeStr.pop_back();
        return timeStr;
    }

public:
    Logger(const std::string& filename, LogLevel level = INFO) : currentLevel(level) {
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
        currentLevel = level;
    }

    void log(LogLevel level, const std::string& message) {
        if (level < currentLevel) return;

        std::string logEntry = "[" + getCurrentTime() + "] [" + levelToString(level) + "] " + message;
        
        if (logFile.is_open()) {
            logFile << logEntry << std::endl;
        }
        
        if (level >= WARNING) {
            std::cerr << logEntry << std::endl;
        } else {
            std::cout << logEntry << std::endl;
        }
    }

    void debug(const std::string& message) { log(DEBUG, message); }
    void info(const std::string& message) { log(INFO, message); }
    void warning(const std::string& message) { log(WARNING, message); }
    void error(const std::string& message) { log(ERROR, message); }
};