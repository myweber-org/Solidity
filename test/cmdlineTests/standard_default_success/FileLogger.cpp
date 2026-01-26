
#include <fstream>
#include <iostream>
#include <mutex>
#include <chrono>
#include <iomanop>

class FileLogger {
public:
    enum class LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    static FileLogger& getInstance(const std::string& filename = "app.log") {
        static FileLogger instance(filename);
        return instance;
    }

    void log(LogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        logFile << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        logFile << '.' << std::setfill('0') << std::setw(3) << ms.count() << " ";
        
        switch(level) {
            case LogLevel::DEBUG: logFile << "[DEBUG] "; break;
            case LogLevel::INFO: logFile << "[INFO] "; break;
            case LogLevel::WARNING: logFile << "[WARNING] "; break;
            case LogLevel::ERROR: logFile << "[ERROR] "; break;
        }
        
        logFile << message << std::endl;
        logFile.flush();
    }

    void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    void info(const std::string& message) { log(LogLevel::INFO, message); }
    void warning(const std::string& message) { log(LogLevel::WARNING, message); }
    void error(const std::string& message) { log(LogLevel::ERROR, message); }

private:
    FileLogger(const std::string& filename) {
        logFile.open(filename, std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Failed to open log file: " + filename);
        }
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    FileLogger(const FileLogger&) = delete;
    FileLogger& operator=(const FileLogger&) = delete;

    std::ofstream logFile;
    std::mutex logMutex;
};