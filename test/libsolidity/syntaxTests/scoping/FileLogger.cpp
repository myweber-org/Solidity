
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <mutex>

namespace fs = std::filesystem;

class FileLogger {
public:
    explicit FileLogger(const std::string& baseName, size_t maxFileSize = 1024 * 1024, int maxFiles = 5)
        : baseFilename(baseName), maxSize(maxFileSize), maxFileCount(maxFiles) {
        openCurrentFile();
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        if (!logFile.is_open()) {
            return;
        }

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        logFile << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] " << message << std::endl;

        if (logFile.tellp() > maxSize) {
            rotateFiles();
        }
    }

private:
    void openCurrentFile() {
        std::string filename = baseFilename + ".log";
        logFile.open(filename, std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Failed to open log file: " + filename);
        }
    }

    void rotateFiles() {
        logFile.close();

        for (int i = maxFileCount - 1; i > 0; --i) {
            std::string oldName = baseFilename + "." + std::to_string(i) + ".log";
            std::string newName = baseFilename + "." + std::to_string(i + 1) + ".log";

            if (fs::exists(oldName)) {
                if (fs::exists(newName)) {
                    fs::remove(newName);
                }
                fs::rename(oldName, newName);
            }
        }

        std::string firstBackup = baseFilename + ".1.log";
        std::string current = baseFilename + ".log";
        if (fs::exists(current)) {
            fs::rename(current, firstBackup);
        }

        openCurrentFile();
    }

    std::string baseFilename;
    size_t maxSize;
    int maxFileCount;
    std::ofstream logFile;
    std::mutex logMutex;
};