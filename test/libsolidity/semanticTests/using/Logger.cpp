#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>

class Logger {
public:
    enum class Level {
        INFO,
        WARNING,
        ERROR
    };

    Logger(const std::string& baseFilename, size_t maxFileSize = 1024 * 1024, int maxFiles = 5)
        : baseFilename_(baseFilename), maxFileSize_(maxFileSize), maxFiles_(maxFiles), currentFileSize_(0) {
        rotateIfNeeded();
        openCurrentFile();
    }

    ~Logger() {
        if (logFile_.is_open()) {
            logFile_.close();
        }
    }

    void log(Level level, const std::string& message) {
        if (!logFile_.is_open()) return;

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        logFile_ << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        logFile_ << '.' << std::setfill('0') << std::setw(3) << ms.count() << " ";
        
        switch (level) {
            case Level::INFO: logFile_ << "[INFO] "; break;
            case Level::WARNING: logFile_ << "[WARNING] "; break;
            case Level::ERROR: logFile_ << "[ERROR] "; break;
        }
        
        logFile_ << message << std::endl;
        logFile_.flush();
        
        currentFileSize_ = logFile_.tellp();
        rotateIfNeeded();
    }

    void info(const std::string& message) {
        log(Level::INFO, message);
    }

    void warning(const std::string& message) {
        log(Level::WARNING, message);
    }

    void error(const std::string& message) {
        log(Level::ERROR, message);
    }

private:
    void openCurrentFile() {
        std::string filename = baseFilename_ + "_0.log";
        logFile_.open(filename, std::ios::app);
        if (logFile_.is_open()) {
            logFile_.seekp(0, std::ios::end);
            currentFileSize_ = logFile_.tellp();
        }
    }

    void rotateIfNeeded() {
        if (currentFileSize_ < maxFileSize_) return;
        
        if (logFile_.is_open()) {
            logFile_.close();
        }

        for (int i = maxFiles_ - 1; i > 0; --i) {
            std::string oldName = baseFilename_ + "_" + std::to_string(i-1) + ".log";
            std::string newName = baseFilename_ + "_" + std::to_string(i) + ".log";
            
            if (std::filesystem::exists(oldName)) {
                std::filesystem::rename(oldName, newName);
            }
        }

        std::string firstFile = baseFilename_ + "_0.log";
        if (std::filesystem::exists(firstFile)) {
            std::filesystem::remove(firstFile);
        }

        openCurrentFile();
    }

    std::string baseFilename_;
    size_t maxFileSize_;
    int maxFiles_;
    size_t currentFileSize_;
    std::ofstream logFile_;
};
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <mutex>

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

    std::string getCurrentTime() {
        time_t now = time(0);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
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
    Logger(const std::string& filename, LogLevel level = LogLevel::INFO, bool consoleOutput = true)
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
        std::string logEntry = "[" + getCurrentTime() + "] [" + levelToString(level) + "] " + message;

        if (outputToConsole) {
            std::cout << logEntry << std::endl;
        }

        if (logFile.is_open()) {
            logFile << logEntry << std::endl;
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