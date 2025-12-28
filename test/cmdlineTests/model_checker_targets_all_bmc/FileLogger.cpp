
#include <fstream>
#include <iostream>
#include <chrono>
#include <filesystem>
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
    std::string currentFilePath;

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
        return ss.str();
    }

    void rotateIfNeeded() {
        if (currentSize >= maxFileSize) {
            currentStream.close();
            std::string newName = logDir + "/" + baseName + "_" + getTimestamp() + ".log";
            fs::rename(currentFilePath, newName);
            openNewFile();
            cleanupOldFiles();
        }
    }

    void openNewFile() {
        currentFilePath = logDir + "/" + baseName + ".log";
        currentStream.open(currentFilePath, std::ios::app);
        currentSize = fs::exists(currentFilePath) ? fs::file_size(currentFilePath) : 0;
    }

    void cleanupOldFiles() {
        std::vector<fs::path> logFiles;
        for (const auto& entry : fs::directory_iterator(logDir)) {
            if (entry.is_regular_file() && entry.path().filename().string().find(baseName) != std::string::npos) {
                logFiles.push_back(entry.path());
            }
        }
        std::sort(logFiles.begin(), logFiles.end(),
                  [](const fs::path& a, const fs::path& b) {
                      return fs::last_write_time(a) > fs::last_write_time(b);
                  });
        while (logFiles.size() > static_cast<size_t>(maxFiles)) {
            fs::remove(logFiles.back());
            logFiles.pop_back();
        }
    }

public:
    FileLogger(const std::string& dir, const std::string& name, size_t maxSize = 1024 * 1024, int maxCount = 5)
        : logDir(dir), baseName(name), maxFileSize(maxSize), maxFiles(maxCount), currentSize(0) {
        if (!fs::exists(logDir)) {
            fs::create_directories(logDir);
        }
        openNewFile();
    }

    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }

    void log(const std::string& message) {
        std::string entry = "[" + getTimestamp() + "] " + message + "\n";
        currentStream << entry;
        currentStream.flush();
        currentSize += entry.size();
        rotateIfNeeded();
    }
};

int main() {
    FileLogger logger("./logs", "app");
    for (int i = 0; i < 100; ++i) {
        logger.log("Test log entry number " + std::to_string(i));
    }
    return 0;
}