
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>

class FileLogger {
public:
    explicit FileLogger(const std::string& baseName, size_t maxSize = 1048576, int maxFiles = 5)
        : baseFilename(baseName), maxFileSize(maxSize), maxBackupFiles(maxFiles), currentSize(0) {
        openLogFile();
    }

    void log(const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::string timestamp = std::ctime(&time);
        timestamp.pop_back();

        std::string logEntry = "[" + timestamp + "] " + message + "\n";
        currentSize += logEntry.size();

        if (currentSize > maxFileSize) {
            rotateLogFiles();
            openLogFile();
            currentSize = logEntry.size();
        }

        logFile << logEntry;
        logFile.flush();
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

private:
    void openLogFile() {
        logFile.open(baseFilename, std::ios::app);
    }

    void rotateLogFiles() {
        if (logFile.is_open()) {
            logFile.close();
        }

        for (int i = maxBackupFiles - 1; i > 0; --i) {
            std::string oldName = baseFilename + "." + std::to_string(i);
            std::string newName = baseFilename + "." + std::to_string(i + 1);
            if (std::filesystem::exists(oldName)) {
                std::filesystem::rename(oldName, newName);
            }
        }

        std::string firstBackup = baseFilename + ".1";
        std::filesystem::rename(baseFilename, firstBackup);
    }

    std::ofstream logFile;
    std::string baseFilename;
    size_t maxFileSize;
    int maxBackupFiles;
    size_t currentSize;
};