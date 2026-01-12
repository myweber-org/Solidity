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