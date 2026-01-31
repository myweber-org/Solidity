#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <mutex>
#include <sstream>

namespace fs = std::filesystem;

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
        rotateIfNeeded();
    }

    void setLevel(Level level) {
        std::lock_guard<std::mutex> lock(mutex_);
        currentLevel_ = level;
    }

    void log(Level level, const std::string& message) {
        if (level < currentLevel_) return;

        std::lock_guard<std::mutex> lock(mutex_);
        rotateIfNeeded();

        std::ofstream file(baseFilename_, std::ios::app);
        if (file.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

            file << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
            file << "." << std::setfill('0') << std::setw(3) << ms.count() << "] ";
            file << levelToString(level) << ": " << message << std::endl;
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

    void rotateIfNeeded() {
        if (!fs::exists(baseFilename_)) return;

        auto size = fs::file_size(baseFilename_);
        if (size < maxFileSize_) return;

        for (int i = maxFiles_ - 1; i > 0; --i) {
            std::string oldName = baseFilename_ + "." + std::to_string(i);
            std::string newName = baseFilename_ + "." + std::to_string(i + 1);
            if (fs::exists(oldName)) {
                fs::rename(oldName, newName);
            }
        }

        fs::rename(baseFilename_, baseFilename_ + ".1");
    }
};