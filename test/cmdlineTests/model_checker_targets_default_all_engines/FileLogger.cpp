#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

class FileLogger {
public:
    explicit FileLogger(const std::string& baseName, size_t maxSize = 1024 * 1024, int maxFiles = 5)
        : baseFilename(baseName), maxFileSize(maxSize), maxBackupFiles(maxFiles), currentSize(0) {
        openLogFile();
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(const std::string& message) {
        if (!logFile.is_open()) {
            return;
        }

        std::string entry = getCurrentTimestamp() + " - " + message + "\n";
        size_t entrySize = entry.size();

        if (currentSize + entrySize > maxFileSize) {
            rotateLogFiles();
            openLogFile();
        }

        logFile << entry;
        logFile.flush();
        currentSize += entrySize;
    }

private:
    void openLogFile() {
        logFile.open(baseFilename, std::ios::app);
        if (logFile.is_open()) {
            logFile.seekp(0, std::ios::end);
            currentSize = static_cast<size_t>(logFile.tellp());
        }
    }

    void rotateLogFiles() {
        if (logFile.is_open()) {
            logFile.close();
        }

        for (int i = maxBackupFiles - 1; i > 0; --i) {
            fs::path oldFile = baseFilename + "." + std::to_string(i);
            fs::path newFile = baseFilename + "." + std::to_string(i + 1);

            if (fs::exists(oldFile)) {
                if (fs::exists(newFile)) {
                    fs::remove(newFile);
                }
                fs::rename(oldFile, newFile);
            }
        }

        fs::path firstBackup = baseFilename + ".1";
        if (fs::exists(firstBackup)) {
            fs::remove(firstBackup);
        }
        fs::rename(baseFilename, firstBackup);
    }

    std::string getCurrentTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

    std::string baseFilename;
    size_t maxFileSize;
    int maxBackupFiles;
    std::ofstream logFile;
    size_t currentSize;
};

void exampleUsage() {
    FileLogger logger("application.log", 1024, 3);

    for (int i = 0; i < 100; ++i) {
        logger.log("Log entry number: " + std::to_string(i));
    }

    std::cout << "Logging completed. Check application.log and backup files." << std::endl;
}