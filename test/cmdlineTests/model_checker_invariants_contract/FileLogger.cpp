
#include <fstream>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <sstream>
#include <iomanip>

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
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");
        return ss.str();
    }

    void rotateIfNeeded() {
        if (currentSize >= maxFileSize) {
            currentStream.close();
            std::string newName = logDir + "/" + baseName + "_" + generateTimestamp() + ".log";
            fs::rename(logDir + "/current.log", newName);
            openCurrentFile();
            cleanupOldFiles();
        }
    }

    void openCurrentFile() {
        std::string currentPath = logDir + "/current.log";
        currentStream.open(currentPath, std::ios::app);
        currentSize = fs::file_size(currentPath);
    }

    void cleanupOldFiles() {
        std::vector<fs::path> logFiles;
        for (const auto& entry : fs::directory_iterator(logDir)) {
            if (entry.path().extension() == ".log" && entry.path().filename().string().find(baseName) != std::string::npos) {
                logFiles.push_back(entry.path());
            }
        }
        std::sort(logFiles.begin(), logFiles.end(), [](const fs::path& a, const fs::path& b) {
            return fs::last_write_time(a) > fs::last_write_time(b);
        });
        while (logFiles.size() > static_cast<size_t>(maxFiles)) {
            fs::remove(logFiles.back());
            logFiles.pop_back();
        }
    }

public:
    FileLogger(const std::string& dir, const std::string& name, size_t maxSize = 1048576, int maxCount = 10)
        : logDir(dir), baseName(name), maxFileSize(maxSize), maxFiles(maxCount), currentSize(0) {
        if (!fs::exists(logDir)) {
            fs::create_directories(logDir);
        }
        openCurrentFile();
    }

    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }

    void log(const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "[%Y-%m-%d %H:%M:%S] ") << message << std::endl;
        std::string logEntry = ss.str();
        currentStream << logEntry;
        currentStream.flush();
        currentSize += logEntry.size();
        rotateIfNeeded();
    }
};

int main() {
    FileLogger logger("./logs", "app", 1024, 5);
    for (int i = 0; i < 100; ++i) {
        logger.log("Log entry number: " + std::to_string(i));
    }
    return 0;
}