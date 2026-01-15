#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>

class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    Logger(const std::string& baseFilename, size_t maxFileSize = 1024 * 1024, int maxFiles = 5)
        : baseFilename_(baseFilename), maxFileSize_(maxFileSize), maxFiles_(maxFiles), currentLevel_(Level::INFO) {
        openLogFile();
    }

    ~Logger() {
        if (logFile_.is_open()) {
            logFile_.close();
        }
    }

    void setLogLevel(Level level) {
        currentLevel_ = level;
    }

    void log(Level level, const std::string& message) {
        if (level < currentLevel_) return;

        std::lock_guard<std::mutex> lock(logMutex_);
        if (!logFile_.is_open()) {
            std::cerr << "Log file is not open!" << std::endl;
            return;
        }

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        logFile_ << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        logFile_ << "." << std::setfill('0') << std::setw(3) << ms.count() << "] ";
        logFile_ << levelToString(level) << ": " << message << std::endl;

        if (logFile_.tellp() > maxFileSize_) {
            rotateLogFiles();
        }
    }

    void debug(const std::string& message) { log(Level::DEBUG, message); }
    void info(const std::string& message) { log(Level::INFO, message); }
    void warning(const std::string& message) { log(Level::WARNING, message); }
    void error(const std::string& message) { log(Level::ERROR, message); }

private:
    std::string baseFilename_;
    size_t maxFileSize_;
    int maxFiles_;
    Level currentLevel_;
    std::ofstream logFile_;
    std::mutex logMutex_;

    void openLogFile() {
        std::string filename = baseFilename_ + ".log";
        logFile_.open(filename, std::ios::app);
        if (!logFile_.is_open()) {
            std::cerr << "Failed to open log file: " << filename << std::endl;
        }
    }

    void rotateLogFiles() {
        logFile_.close();

        for (int i = maxFiles_ - 1; i > 0; --i) {
            std::string oldName = baseFilename_ + "." + std::to_string(i) + ".log";
            std::string newName = baseFilename_ + "." + std::to_string(i + 1) + ".log";

            if (std::filesystem::exists(oldName)) {
                std::filesystem::rename(oldName, newName);
            }
        }

        std::string currentLog = baseFilename_ + ".log";
        std::string firstBackup = baseFilename_ + ".1.log";
        if (std::filesystem::exists(currentLog)) {
            std::filesystem::rename(currentLog, firstBackup);
        }

        openLogFile();
    }

    std::string levelToString(Level level) {
        switch (level) {
            case Level::DEBUG: return "DEBUG";
            case Level::INFO: return "INFO";
            case Level::WARNING: return "WARNING";
            case Level::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }
};