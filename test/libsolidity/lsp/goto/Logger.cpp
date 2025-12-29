#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

class Logger {
public:
    enum class LogLevel {
        INFO,
        WARNING,
        ERROR
    };

    Logger(const std::string& filename) : logFile(filename, std::ios::app) {
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

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();

        std::string levelStr;
        switch (level) {
            case LogLevel::INFO: levelStr = "INFO"; break;
            case LogLevel::WARNING: levelStr = "WARNING"; break;
            case LogLevel::ERROR: levelStr = "ERROR"; break;
        }

        logFile << "[" << ss.str() << "] [" << levelStr << "] " << message << std::endl;
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
};