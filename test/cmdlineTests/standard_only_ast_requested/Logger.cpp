#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>

class Logger {
public:
    enum class Level { DEBUG, INFO, WARNING, ERROR };

    Logger(const std::string& baseFilename, size_t maxFileSize = 1024 * 1024, int maxFiles = 5)
        : baseFilename_(baseFilename), maxFileSize_(maxFileSize), maxFiles_(maxFiles), currentSize_(0) {
        rotateIfNeeded();
        openCurrentFile();
    }

    ~Logger() {
        if (file_.is_open()) {
            file_.close();
        }
    }

    void log(Level level, const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::string levelStr;
        switch (level) {
            case Level::DEBUG: levelStr = "DEBUG"; break;
            case Level::INFO: levelStr = "INFO"; break;
            case Level::WARNING: levelStr = "WARNING"; break;
            case Level::ERROR: levelStr = "ERROR"; break;
        }

        std::ostringstream logLine;
        logLine << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        logLine << '.' << std::setfill('0') << std::setw(3) << ms.count();
        logLine << " [" << levelStr << "] " << message << std::endl;

        std::string logEntry = logLine.str();
        file_ << logEntry;
        file_.flush();
        currentSize_ += logEntry.size();

        rotateIfNeeded();
    }

    void debug(const std::string& message) { log(Level::DEBUG, message); }
    void info(const std::string& message) { log(Level::INFO, message); }
    void warning(const std::string& message) { log(Level::WARNING, message); }
    void error(const std::string& message) { log(Level::ERROR, message); }

private:
    void openCurrentFile() {
        std::string filename = baseFilename_ + ".log";
        file_.open(filename, std::ios::app);
        if (file_.is_open()) {
            file_.seekp(0, std::ios::end);
            currentSize_ = file_.tellp();
        }
    }

    void rotateIfNeeded() {
        if (currentSize_ >= maxFileSize_) {
            file_.close();

            for (int i = maxFiles_ - 1; i > 0; --i) {
                std::string oldName = baseFilename_ + "." + std::to_string(i) + ".log";
                std::string newName = baseFilename_ + "." + std::to_string(i + 1) + ".log";
                if (std::filesystem::exists(oldName)) {
                    std::filesystem::rename(oldName, newName);
                }
            }

            std::string currentName = baseFilename_ + ".log";
            std::string firstBackup = baseFilename_ + ".1.log";
            if (std::filesystem::exists(currentName)) {
                std::filesystem::rename(currentName, firstBackup);
            }

            openCurrentFile();
        }
    }

    std::string baseFilename_;
    size_t maxFileSize_;
    int maxFiles_;
    size_t currentSize_;
    std::ofstream file_;
};