#include <fstream>
#include <string>
#include <chrono>
#include <filesystem>
#include <mutex>

namespace fs = std::filesystem;

class FileLogger {
public:
    enum class Level { DEBUG, INFO, WARNING, ERROR };

    FileLogger(const std::string& basePath, size_t maxSize = 1048576, Level minLevel = Level::INFO)
        : basePath_(basePath), maxSize_(maxSize), minLevel_(minLevel), currentSize_(0) {
        openLogFile();
    }

    ~FileLogger() {
        if (logFile_.is_open()) {
            logFile_.close();
        }
    }

    void log(Level level, const std::string& message) {
        if (level < minLevel_) return;

        std::lock_guard<std::mutex> lock(mutex_);
        checkRotation();

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        char timeStr[20];
        std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", std::localtime(&time));

        std::string levelStr;
        switch (level) {
            case Level::DEBUG: levelStr = "DEBUG"; break;
            case Level::INFO: levelStr = "INFO"; break;
            case Level::WARNING: levelStr = "WARNING"; break;
            case Level::ERROR: levelStr = "ERROR"; break;
        }

        std::string logEntry = "[" + std::string(timeStr) + "] [" + levelStr + "] " + message + "\n";
        logFile_ << logEntry;
        logFile_.flush();
        currentSize_ += logEntry.size();
    }

    void debug(const std::string& message) { log(Level::DEBUG, message); }
    void info(const std::string& message) { log(Level::INFO, message); }
    void warning(const std::string& message) { log(Level::WARNING, message); }
    void error(const std::string& message) { log(Level::ERROR, message); }

private:
    void openLogFile() {
        std::string filename = basePath_ + ".log";
        logFile_.open(filename, std::ios::app);
        if (logFile_.is_open()) {
            currentSize_ = fs::file_size(filename);
        }
    }

    void checkRotation() {
        if (currentSize_ >= maxSize_) {
            logFile_.close();
            rotateFiles();
            openLogFile();
        }
    }

    void rotateFiles() {
        for (int i = 9; i >= 0; --i) {
            std::string oldName = basePath_ + (i == 0 ? ".log" : "." + std::to_string(i) + ".log");
            std::string newName = basePath_ + "." + std::to_string(i + 1) + ".log";

            if (fs::exists(oldName)) {
                if (i == 9) {
                    fs::remove(oldName);
                } else {
                    fs::rename(oldName, newName);
                }
            }
        }
    }

    std::string basePath_;
    size_t maxSize_;
    Level minLevel_;
    std::ofstream logFile_;
    size_t currentSize_;
    std::mutex mutex_;
};