
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <filesystem>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

class FileLogger {
private:
    std::ofstream logFile;
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
        return ss.str();
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
    FileLogger(const std::string& filename) {
        std::filesystem::path filePath(filename);
        std::filesystem::create_directories(filePath.parent_path());
        logFile.open(filename, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file: " << filename << std::endl;
        }
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(LogLevel level, const std::string& message) {
        if (!logFile.is_open()) return;
        
        logFile << "[" << getCurrentTimestamp() << "] "
                << "[" << levelToString(level) << "] "
                << message << std::endl;
        
        if (level == LogLevel::ERROR) {
            logFile.flush();
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

int main() {
    FileLogger logger("logs/application.log");
    
    logger.info("Application started");
    logger.warning("Low disk space detected");
    logger.error("Failed to connect to database");
    
    logger.info("Application shutdown");
    
    return 0;
}