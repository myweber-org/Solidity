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
            file << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] ";
            file << "[" << levelToString(level) << "] ";
            file << message << std::endl;
        }
    }

    void debug(const std::string& message) { log(Level::DEBUG, message); }
    void info(const std::string& message) { log(Level::INFO, message); }
    void warning(const std::string& message) { log(Level::WARNING, message); }
    void error(const std::string& message) { log(Level::ERROR, message); }

private:
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
        if (fs::exists(baseFilename_)) {
            auto size = fs::file_size(baseFilename_);
            if (size >= maxFileSize_) {
                rotateFiles();
            }
        }
    }

    void rotateFiles() {
        for (int i = maxFiles_ - 1; i > 0; --i) {
            std::string oldName = baseFilename_ + "." + std::to_string(i);
            std::string newName = baseFilename_ + "." + std::to_string(i + 1);
            if (fs::exists(oldName)) {
                fs::rename(oldName, newName);
            }
        }
        std::string firstBackup = baseFilename_ + ".1";
        fs::rename(baseFilename_, firstBackup);
    }

    std::string baseFilename_;
    size_t maxFileSize_;
    int maxFiles_;
    Level currentLevel_;
    std::mutex mutex_;
};

void exampleUsage() {
    Logger logger("application.log", 1024 * 1024, 3);
    logger.setLevel(Logger::Level::DEBUG);

    logger.debug("Starting application");
    logger.info("Initialization complete");
    logger.warning("Resource usage is high");
    logger.error("Failed to connect to database");

    for (int i = 0; i < 1000; ++i) {
        logger.info("Processing item " + std::to_string(i));
    }
}

int main() {
    exampleUsage();
    return 0;
}