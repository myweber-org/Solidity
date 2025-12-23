#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>

class FileLogger {
private:
    std::string logDir;
    std::string baseName;
    size_t maxFileSize;
    int maxFiles;
    std::ofstream currentStream;
    int currentFileIndex;
    size_t currentSize;

    std::string generateFileName() const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
        return logDir + "/" + baseName + "_" + oss.str() + "_" + std::to_string(currentFileIndex) + ".log";
    }

    void rotateIfNeeded() {
        if (currentSize >= maxFileSize) {
            currentStream.close();
            currentFileIndex++;
            if (currentFileIndex >= maxFiles) {
                currentFileIndex = 0;
            }
            openNewFile();
        }
    }

    void openNewFile() {
        std::string newFileName = generateFileName();
        currentStream.open(newFileName, std::ios::out | std::ios::app);
        if (!currentStream.is_open()) {
            std::cerr << "Failed to open log file: " << newFileName << std::endl;
            return;
        }
        currentSize = 0;
        logInternal("INFO", "Log file created: " + newFileName);
    }

    void logInternal(const std::string& level, const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::ostringstream timestamp;
        timestamp << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");

        std::string logEntry = "[" + timestamp.str() + "] [" + level + "] " + message + "\n";
        currentStream << logEntry;
        currentStream.flush();
        currentSize += logEntry.size();
    }

public:
    FileLogger(const std::string& dir = "logs", const std::string& name = "app",
               size_t maxSize = 1024 * 1024, int maxCount = 5)
        : logDir(dir), baseName(name), maxFileSize(maxSize), maxFiles(maxCount),
          currentFileIndex(0), currentSize(0) {

        std::filesystem::create_directories(logDir);
        openNewFile();
    }

    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }

    void info(const std::string& message) {
        rotateIfNeeded();
        logInternal("INFO", message);
    }

    void warn(const std::string& message) {
        rotateIfNeeded();
        logInternal("WARN", message);
    }

    void error(const std::string& message) {
        rotateIfNeeded();
        logInternal("ERROR", message);
    }

    void debug(const std::string& message) {
        rotateIfNeeded();
        logInternal("DEBUG", message);
    }
};