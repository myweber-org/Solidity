
#include <fstream>
#include <iostream>
#include <mutex>
#include <chrono>
#include <iomanip>

class FileLogger {
public:
    enum class Level { DEBUG, INFO, WARNING, ERROR };

    FileLogger(const std::string& filename) : logFile(filename, std::ios::app) {
        if (!logFile.is_open()) {
            throw std::runtime_error("Failed to open log file");
        }
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(Level level, const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        logFile << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] ";
        
        switch (level) {
            case Level::DEBUG: logFile << "[DEBUG] "; break;
            case Level::INFO: logFile << "[INFO] "; break;
            case Level::WARNING: logFile << "[WARNING] "; break;
            case Level::ERROR: logFile << "[ERROR] "; break;
        }
        
        logFile << message << std::endl;
        logFile.flush();
    }

    void debug(const std::string& message) { log(Level::DEBUG, message); }
    void info(const std::string& message) { log(Level::INFO, message); }
    void warning(const std::string& message) { log(Level::WARNING, message); }
    void error(const std::string& message) { log(Level::ERROR, message); }

private:
    std::ofstream logFile;
    std::mutex logMutex;
};