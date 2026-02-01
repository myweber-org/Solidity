
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <mutex>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class FileLogger {
private:
    std::ofstream logFile;
    std::string baseFilename;
    size_t maxFileSize;
    int maxBackupFiles;
    std::mutex writeMutex;
    LogLevel currentLevel;

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    std::string levelToString(LogLevel level) {
        switch(level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

    void rotateIfNeeded() {
        if (!logFile.is_open()) return;

        logFile.seekp(0, std::ios::end);
        size_t currentSize = logFile.tellp();

        if (currentSize >= maxFileSize) {
            logFile.close();

            for (int i = maxBackupFiles - 1; i > 0; --i) {
                std::string oldName = baseFilename + "." + std::to_string(i);
                std::string newName = baseFilename + "." + std::to_string(i + 1);

                if (std::filesystem::exists(oldName)) {
                    std::filesystem::rename(oldName, newName);
                }
            }

            std::string firstBackup = baseFilename + ".1";
            std::filesystem::rename(baseFilename, firstBackup);

            openLogFile();
        }
    }

    void openLogFile() {
        logFile.open(baseFilename, std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Cannot open log file: " + baseFilename);
        }
    }

public:
    FileLogger(const std::string& filename, size_t maxSize = 1048576, int backups = 5, LogLevel level = LogLevel::INFO)
        : baseFilename(filename), maxFileSize(maxSize), maxBackupFiles(backups), currentLevel(level) {
        openLogFile();
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void setLogLevel(LogLevel level) {
        currentLevel = level;
    }

    void log(LogLevel level, const std::string& message) {
        if (level < currentLevel) return;

        std::lock_guard<std::mutex> lock(writeMutex);
        
        rotateIfNeeded();
        
        logFile << getTimestamp() << " [" << levelToString(level) << "] " << message << std::endl;
        logFile.flush();
    }

    void debug(const std::string& message) {
        log(LogLevel::DEBUG, message);
    }

    void info(const std::string& message) {
        log(LogLevel::INFO, message);
    }

    void warning(const std::string& message) {
        log(LogLevel::WARNING, message);
    }

    void error(const std::string& message) {
        log(LogLevel::ERROR, message);
    }
};