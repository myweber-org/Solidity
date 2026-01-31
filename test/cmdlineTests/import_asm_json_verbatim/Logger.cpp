
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

    Logger(const std::string& baseFilename, size_t maxFileSize = 1024 * 1024, Level minLevel = Level::INFO)
        : baseFilename_(baseFilename), maxFileSize_(maxFileSize), minLevel_(minLevel), currentSize_(0) {
        rotateIfNeeded();
    }

    ~Logger() {
        if (logFile_.is_open()) {
            logFile_.close();
        }
    }

    void log(Level level, const std::string& message) {
        if (level < minLevel_) return;

        std::lock_guard<std::mutex> lock(mutex_);
        if (!logFile_.is_open()) {
            openLogFile();
        }

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        logFile_ << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        logFile_ << '.' << std::setfill('0') << std::setw(3) << ms.count();
        logFile_ << " [" << levelToString(level) << "] " << message << std::endl;

        currentSize_ = logFile_.tellp();
        rotateIfNeeded();
    }

    void debug(const std::string& message) { log(Level::DEBUG, message); }
    void info(const std::string& message) { log(Level::INFO, message); }
    void warning(const std::string& message) { log(Level::WARNING, message); }
    void error(const std::string& message) { log(Level::ERROR, message); }

    void setMinLevel(Level level) { minLevel_ = level; }

private:
    std::string baseFilename_;
    size_t maxFileSize_;
    Level minLevel_;
    std::ofstream logFile_;
    size_t currentSize_;
    std::mutex mutex_;

    std::string levelToString(Level level) {
        switch (level) {
            case Level::DEBUG: return "DEBUG";
            case Level::INFO: return "INFO";
            case Level::WARNING: return "WARNING";
            case Level::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

    void openLogFile() {
        logFile_.open(baseFilename_, std::ios::app);
        if (logFile_.is_open()) {
            logFile_.seekp(0, std::ios::end);
            currentSize_ = logFile_.tellp();
        }
    }

    void rotateIfNeeded() {
        if (currentSize_ >= maxFileSize_) {
            if (logFile_.is_open()) {
                logFile_.close();
            }

            for (int i = 9; i >= 1; --i) {
                std::string oldName = baseFilename_ + "." + std::to_string(i);
                std::string newName = baseFilename_ + "." + std::to_string(i + 1);
                if (fs::exists(oldName)) {
                    fs::rename(oldName, newName);
                }
            }

            std::string firstBackup = baseFilename_ + ".1";
            fs::rename(baseFilename_, firstBackup);

            openLogFile();
        }
    }
};