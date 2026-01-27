#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

class FileLogger {
private:
    std::string logDir;
    std::string baseName;
    size_t maxFileSize;
    int maxFiles;
    std::ofstream currentStream;
    size_t currentSize;

    std::string generateTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
        return ss.str();
    }

    void rotateIfNeeded() {
        if (currentSize >= maxFileSize && currentStream.is_open()) {
            currentStream.close();
            std::string newName = logDir + "/" + baseName + "_" + generateTimestamp() + ".log";
            fs::rename(logDir + "/" + baseName + ".log", newName);
            cleanupOldFiles();
            openCurrentLog();
        }
    }

    void cleanupOldFiles() {
        std::vector<fs::path> logFiles;
        for (const auto& entry : fs::directory_iterator(logDir)) {
            if (entry.is_regular_file() && entry.path().filename().string().find(baseName) == 0) {
                logFiles.push_back(entry.path());
            }
        }
        std::sort(logFiles.begin(), logFiles.end(), [](const fs::path& a, const fs::path& b) {
            return fs::last_write_time(a) > fs::last_write_time(b);
        });
        while (logFiles.size() > maxFiles) {
            fs::remove(logFiles.back());
            logFiles.pop_back();
        }
    }

    void openCurrentLog() {
        std::string currentPath = logDir + "/" + baseName + ".log";
        currentStream.open(currentPath, std::ios::app);
        currentSize = fs::file_size(currentPath);
    }

public:
    FileLogger(const std::string& directory, const std::string& name, size_t maxSize = 1024 * 1024, int maxCount = 10)
        : logDir(directory), baseName(name), maxFileSize(maxSize), maxFiles(maxCount), currentSize(0) {
        if (!fs::exists(logDir)) {
            fs::create_directories(logDir);
        }
        openCurrentLog();
    }

    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }

    void log(const std::string& message) {
        rotateIfNeeded();
        if (currentStream.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            currentStream << std::put_time(std::localtime(&time), "[%Y-%m-%d %H:%M:%S] ") << message << std::endl;
            currentSize += message.length() + 30;
        }
    }
};

int main() {
    FileLogger logger("./logs", "app", 1024, 5);
    for (int i = 0; i < 100; ++i) {
        logger.log("Log entry number: " + std::to_string(i));
    }
    return 0;
}