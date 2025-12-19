#include <fstream>
#include <string>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

class FileLogger {
private:
    std::string baseFilename;
    std::string logDirectory;
    size_t maxFileSize;
    int maxBackupFiles;
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
        if (currentSize >= maxFileSize) {
            currentStream.close();
            std::string timestamp = generateTimestamp();
            std::string newName = logDirectory + "/" + baseFilename + "_" + timestamp + ".log";
            fs::rename(logDirectory + "/" + baseFilename + ".log", newName);

            cleanupOldFiles();
            openCurrentFile();
        }
    }

    void cleanupOldFiles() {
        std::vector<fs::path> logFiles;
        for (const auto& entry : fs::directory_iterator(logDirectory)) {
            if (entry.is_regular_file() && entry.path().filename().string().find(baseFilename) == 0) {
                logFiles.push_back(entry.path());
            }
        }

        std::sort(logFiles.begin(), logFiles.end(),
                  [](const fs::path& a, const fs::path& b) {
                      return fs::last_write_time(a) > fs::last_write_time(b);
                  });

        while (logFiles.size() > static_cast<size_t>(maxBackupFiles)) {
            fs::remove(logFiles.back());
            logFiles.pop_back();
        }
    }

    void openCurrentFile() {
        std::string currentPath = logDirectory + "/" + baseFilename + ".log";
        currentStream.open(currentPath, std::ios::app);
        currentSize = fs::file_size(currentPath);
    }

public:
    FileLogger(const std::string& dir, const std::string& base, size_t maxSize = 1048576, int maxBackups = 5)
        : logDirectory(dir), baseFilename(base), maxFileSize(maxSize), maxBackupFiles(maxBackups), currentSize(0) {
        if (!fs::exists(logDirectory)) {
            fs::create_directories(logDirectory);
        }
        openCurrentFile();
    }

    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        rotateIfNeeded();

        std::string timestamp = generateTimestamp();
        std::string logEntry = "[" + timestamp + "] " + message + "\n";

        currentStream << logEntry;
        currentStream.flush();
        currentSize += logEntry.size();
    }

private:
    std::mutex logMutex;
};