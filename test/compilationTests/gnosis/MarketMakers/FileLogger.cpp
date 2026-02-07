
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <mutex>

namespace fs = std::filesystem;

class FileLogger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    FileLogger(const std::string& basePath, Level minLevel = Level::INFO, size_t maxFileSize = 1048576, int maxFiles = 5)
        : basePath_(basePath), minLevel_(minLevel), maxFileSize_(maxFileSize), maxFiles_(maxFiles) {
        openCurrentFile();
    }

    ~FileLogger() {
        if (file_.is_open()) {
            file_.close();
        }
    }

    void log(Level level, const std::string& message) {
        if (level < minLevel_) return;

        std::lock_guard<std::mutex> lock(mutex_);
        checkRotation();
        
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        file_ << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        file_ << '.' << std::setfill('0') << std::setw(3) << ms.count();
        file_ << " [" << levelToString(level) << "] " << message << std::endl;
    }

    void debug(const std::string& message) { log(Level::DEBUG, message); }
    void info(const std::string& message) { log(Level::INFO, message); }
    void warning(const std::string& message) { log(Level::WARNING, message); }
    void error(const std::string& message) { log(Level::ERROR, message); }

private:
    void openCurrentFile() {
        currentFile_ = basePath_ + "_" + getCurrentTimestamp() + ".log";
        file_.open(currentFile_, std::ios::app);
        if (!file_.is_open()) {
            throw std::runtime_error("Cannot open log file: " + currentFile_);
        }
    }

    void checkRotation() {
        if (!file_.is_open()) return;
        
        file_.seekp(0, std::ios::end);
        size_t currentSize = file_.tellp();
        
        if (currentSize >= maxFileSize_) {
            file_.close();
            rotateFiles();
            openCurrentFile();
        }
    }

    void rotateFiles() {
        for (int i = maxFiles_ - 1; i >= 0; --i) {
            std::string oldName = basePath_ + "_" + getTimestampForRotation(i) + ".log";
            std::string newName = basePath_ + "_" + getTimestampForRotation(i + 1) + ".log";
            
            if (fs::exists(oldName)) {
                if (i == maxFiles_ - 1) {
                    fs::remove(oldName);
                } else {
                    fs::rename(oldName, newName);
                }
            }
        }
    }

    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
        return ss.str();
    }

    std::string getTimestampForRotation(int index) {
        if (index == 0) return getCurrentTimestamp();
        
        auto now = std::chrono::system_clock::now();
        auto past = now - std::chrono::hours(24 * index);
        auto time = std::chrono::system_clock::to_time_t(past);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
        return ss.str();
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

    std::string basePath_;
    Level minLevel_;
    size_t maxFileSize_;
    int maxFiles_;
    std::string currentFile_;
    std::ofstream file_;
    std::mutex mutex_;
};