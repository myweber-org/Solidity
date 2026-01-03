#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <mutex>

namespace fs = std::filesystem;

class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    Logger(const std::string& baseFilename, size_t maxFileSize = 1024 * 1024, size_t maxFiles = 5)
        : baseFilename_(baseFilename), maxFileSize_(maxFileSize), maxFiles_(maxFiles), currentLevel_(Level::INFO) {
        openLogFile();
    }

    void setLevel(Level level) {
        std::lock_guard<std::mutex> lock(mutex_);
        currentLevel_ = level;
    }

    void log(Level level, const std::string& message) {
        if (level < currentLevel_) return;

        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        if (!logFile_.is_open()) {
            openLogFile();
        }

        logFile_ << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        logFile_ << '.' << std::setfill('0') << std::setw(3) << ms.count();
        logFile_ << " [" << levelToString(level) << "] " << message << std::endl;

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
    size_t maxFiles_;
    Level currentLevel_;
    std::ofstream logFile_;
    std::mutex mutex_;

    void openLogFile() {
        logFile_.open(baseFilename_, std::ios::app);
        if (!logFile_.is_open()) {
            std::cerr << "Failed to open log file: " << baseFilename_ << std::endl;
        }
    }

    void rotateLogFiles() {
        logFile_.close();

        for (int i = maxFiles_ - 1; i >= 0; --i) {
            std::string oldName = i == 0 ? baseFilename_ : baseFilename_ + "." + std::to_string(i);
            std::string newName = baseFilename_ + "." + std::to_string(i + 1);

            if (fs::exists(oldName)) {
                if (i == maxFiles_ - 1) {
                    fs::remove(oldName);
                } else {
                    fs::rename(oldName, newName);
                }
            }
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

void exampleUsage() {
    Logger logger("application.log", 1024 * 1024, 3);
    logger.setLevel(Logger::Level::DEBUG);

    logger.debug("Debug message for detailed information");
    logger.info("Application started successfully");
    logger.warning("Disk space is running low");
    logger.error("Failed to connect to database");

    for (int i = 0; i < 1000; ++i) {
        logger.info("Log entry number: " + std::to_string(i));
    }
}

int main() {
    exampleUsage();
    return 0;
}