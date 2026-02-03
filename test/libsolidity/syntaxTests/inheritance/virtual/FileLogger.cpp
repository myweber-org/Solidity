
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <mutex>

namespace fs = std::filesystem;

class FileLogger {
public:
    enum class Level { DEBUG, INFO, WARNING, ERROR };

    FileLogger(const std::string& basePath, size_t maxFileSize = 1048576, int maxBackups = 5)
        : basePath_(basePath), maxFileSize_(maxFileSize), maxBackups_(maxBackups), currentLevel_(Level::INFO) {
        openLogFile();
    }

    void setLevel(Level level) {
        std::lock_guard<std::mutex> lock(mutex_);
        currentLevel_ = level;
    }

    void log(Level level, const std::string& message) {
        if (level < currentLevel_) return;

        std::lock_guard<std::mutex> lock(mutex_);
        checkRotation();
        
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        file_ << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        file_ << '.' << std::setfill('0') << std::setw(3) << ms.count();
        file_ << " [" << levelToString(level) << "] " << message << std::endl;
    }

    void debug(const std::string& message) { log(Level::DEBUG, message); }
    void info(const std::string& message) { log(Level::INFO, message); }
    void warning(const std::string& message) { log(Level::WARNING, message); }
    void error(const std::string& message) { log(Level::ERROR, message); }

private:
    void openLogFile() {
        file_.open(basePath_, std::ios::app);
        if (!file_.is_open()) {
            throw std::runtime_error("Cannot open log file: " + basePath_);
        }
    }

    void checkRotation() {
        file_.flush();
        auto size = fs::file_size(basePath_);
        
        if (size >= maxFileSize_) {
            file_.close();
            rotateFiles();
            openLogFile();
        }
    }

    void rotateFiles() {
        for (int i = maxBackups_ - 1; i >= 0; --i) {
            std::string oldName = i == 0 ? basePath_ : basePath_ + "." + std::to_string(i);
            std::string newName = basePath_ + "." + std::to_string(i + 1);
            
            if (fs::exists(oldName)) {
                if (i == maxBackups_ - 1) {
                    fs::remove(oldName);
                } else {
                    fs::rename(oldName, newName);
                }
            }
        }
    }

    std::string levelToString(Level level) {
        switch(level) {
            case Level::DEBUG: return "DEBUG";
            case Level::INFO: return "INFO";
            case Level::WARNING: return "WARN";
            case Level::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

    std::string basePath_;
    size_t maxFileSize_;
    int maxBackups_;
    Level currentLevel_;
    std::ofstream file_;
    std::mutex mutex_;
};