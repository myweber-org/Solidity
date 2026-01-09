#include <fstream>
#include <string>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

class FileLogger {
private:
    std::string logDir;
    std::string baseName;
    size_t maxFileSize;
    int maxFiles;
    std::ofstream currentStream;
    std::string currentFilePath;
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
            std::string newName = baseName + "_" + generateTimestamp() + ".log";
            fs::path newPath = fs::path(logDir) / newName;
            fs::rename(currentFilePath, newPath);
            openNewFile();
            cleanupOldFiles();
        }
    }

    void openNewFile() {
        currentFilePath = (fs::path(logDir) / (baseName + ".log")).string();
        currentStream.open(currentFilePath, std::ios::app);
        currentSize = fs::exists(currentFilePath) ? fs::file_size(currentFilePath) : 0;
    }

    void cleanupOldFiles() {
        std::vector<fs::path> logFiles;
        for (const auto& entry : fs::directory_iterator(logDir)) {
            if (entry.is_regular_file() && entry.path().filename().string().find(baseName) == 0) {
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
    FileLogger(const std::string& directory, const std::string& name,
               size_t maxSize = 1048576, int maxCount = 5)
        : logDir(directory), baseName(name), maxFileSize(maxSize), maxFiles(maxCount), currentSize(0) {
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
        if (!currentStream.is_open()) return;
        rotateIfNeeded();
        std::string entry = "[" + generateTimestamp() + "] " + message + "\n";
        currentStream << entry;
        currentStream.flush();
        currentSize += entry.size();
    }

    void logError(const std::string& errorMessage) {
        log("ERROR: " + errorMessage);
    }

    void logInfo(const std::string& infoMessage) {
        log("INFO: " + infoMessage);
    }
};