
#include <fstream>
#include <string>
#include <chrono>
#include <filesystem>
#include <mutex>
#include <sstream>

namespace fs = std::filesystem;

class FileLogger {
public:
    enum class Level { DEBUG, INFO, WARNING, ERROR };

    FileLogger(const std::string& basePath, size_t maxSize = 1048576, int maxFiles = 5)
        : basePath_(basePath), maxSize_(maxSize), maxFiles_(maxFiles), currentSize_(0) {
        openLogFile();
    }

    void log(Level level, const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!logFile_.is_open()) return;

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        char timeStr[20];
        std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", std::localtime(&time));

        std::string levelStr;
        switch(level) {
            case Level::DEBUG: levelStr = "DEBUG"; break;
            case Level::INFO: levelStr = "INFO"; break;
            case Level::WARNING: levelStr = "WARNING"; break;
            case Level::ERROR: levelStr = "ERROR"; break;
        }

        std::string logEntry = std::string(timeStr) + " [" + levelStr + "] " + message + "\n";
        logFile_ << logEntry;
        logFile_.flush();

        currentSize_ += logEntry.size();
        if (currentSize_ >= maxSize_) {
            rotateLogs();
        }
    }

    void debug(const std::string& message) { log(Level::DEBUG, message); }
    void info(const std::string& message) { log(Level::INFO, message); }
    void warning(const std::string& message) { log(Level::WARNING, message); }
    void error(const std::string& message) { log(Level::ERROR, message); }

private:
    void openLogFile() {
        logFile_.open(basePath_, std::ios::app);
        if (logFile_.is_open()) {
            logFile_.seekp(0, std::ios::end);
            currentSize_ = logFile_.tellp();
        }
    }

    void rotateLogs() {
        logFile_.close();

        for (int i = maxFiles_ - 1; i > 0; --i) {
            std::string oldName = basePath_ + "." + std::to_string(i);
            std::string newName = basePath_ + "." + std::to_string(i + 1);
            
            if (fs::exists(oldName)) {
                if (fs::exists(newName)) {
                    fs::remove(newName);
                }
                fs::rename(oldName, newName);
            }
        }

        if (fs::exists(basePath_)) {
            fs::rename(basePath_, basePath_ + ".1");
        }

        openLogFile();
        currentSize_ = 0;
    }

    std::string basePath_;
    size_t maxSize_;
    int maxFiles_;
    size_t currentSize_;
    std::ofstream logFile_;
    std::mutex mutex_;
};