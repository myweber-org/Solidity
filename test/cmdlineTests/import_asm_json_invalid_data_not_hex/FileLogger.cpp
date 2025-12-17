
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <chrono>
#include <iomanip>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

class FileLogger {
public:
    explicit FileLogger(const std::string& filename) {
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
        std::lock_guard<std::mutex> lock(logMutex);
        if (!logFile.is_open()) return;

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        logFile << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        logFile << '.' << std::setfill('0') << std::setw(3) << ms.count() << " ";

        switch (level) {
            case LogLevel::INFO:
                logFile << "[INFO] ";
                break;
            case LogLevel::WARNING:
                logFile << "[WARNING] ";
                break;
            case LogLevel::ERROR:
                logFile << "[ERROR] ";
                break;
        }

        logFile << message << std::endl;
        logFile.flush();
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

private:
    std::ofstream logFile;
    std::mutex logMutex;
};