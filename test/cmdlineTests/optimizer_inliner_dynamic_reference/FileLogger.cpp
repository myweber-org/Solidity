
#include <fstream>
#include <iostream>
#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

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
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", std::localtime(&time));
        return std::string(buffer);
    }

    void rotateIfNeeded() {
        if (currentSize >= maxFileSize) {
            currentStream.close();
            std::string timestamp = generateTimestamp();
            std::string newName = logDirectory + "/" + baseFilename + "_" + timestamp + ".log";
            fs::rename(logDirectory + "/" + baseFilename + ".log", newName);

            manageBackups();

            currentStream.open(logDirectory + "/" + baseFilename + ".log", std::ios::app);
            currentSize = 0;
        }
    }

    void manageBackups() {
        std::vector<fs::path> backupFiles;
        for (const auto& entry : fs::directory_iterator(logDirectory)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename.find(baseFilename) == 0 && filename != baseFilename + ".log") {
                    backupFiles.push_back(entry.path());
                }
            }
        }

        std::sort(backupFiles.begin(), backupFiles.end(),
                  [](const fs::path& a, const fs::path& b) {
                      return fs::last_write_time(a) < fs::last_write_time(b);
                  });

        while (backupFiles.size() > static_cast<size_t>(maxBackupFiles)) {
            fs::remove(backupFiles.front());
            backupFiles.erase(backupFiles.begin());
        }
    }

public:
    FileLogger(const std::string& dir = "logs",
               const std::string& filename = "app",
               size_t maxSize = 1048576,
               int backups = 5)
        : logDirectory(dir), baseFilename(filename), maxFileSize(maxSize), maxBackupFiles(backups), currentSize(0) {

        if (!fs::exists(logDirectory)) {
            fs::create_directories(logDirectory);
        }

        std::string fullPath = logDirectory + "/" + baseFilename + ".log";
        currentStream.open(fullPath, std::ios::app);
        if (currentStream) {
            currentStream.seekp(0, std::ios::end);
            currentSize = currentStream.tellp();
        }
    }

    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }

    void log(const std::string& message) {
        rotateIfNeeded();

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        char timeBuffer[100];
        std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time));

        std::string logEntry = "[" + std::string(timeBuffer) + "] " + message + "\n";

        if (currentStream.is_open()) {
            currentStream << logEntry;
            currentStream.flush();
            currentSize += logEntry.size();
        }
    }

    void flush() {
        if (currentStream.is_open()) {
            currentStream.flush();
        }
    }
};

int main() {
    FileLogger logger("mylogs", "testlog", 1024, 3);

    for (int i = 1; i <= 50; ++i) {
        logger.log("This is log message number " + std::to_string(i));
        std::cout << "Logged message " << i << std::endl;
    }

    return 0;
}