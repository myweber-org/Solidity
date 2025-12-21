
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>

class FileLogger {
public:
    explicit FileLogger(const std::string& baseName, size_t maxFileSize = 1048576, int maxFiles = 5)
        : baseFileName(baseName), maxSize(maxFileSize), maxFileCount(maxFiles), currentSize(0) {
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
        logFile << entry;
        logFile.flush();
        currentSize += entry.size();

        if (currentSize >= maxSize) {
            rotateLogFiles();
        }
    }

private:
    void openLogFile() {
        std::string fileName = baseFileName + ".log";
        logFile.open(fileName, std::ios::app);
        if (logFile.is_open()) {
            currentSize = std::filesystem::file_size(fileName);
        }
    }

    void rotateLogFiles() {
        logFile.close();

        for (int i = maxFileCount - 1; i > 0; --i) {
            std::string oldName = baseFileName + "." + std::to_string(i) + ".log";
            std::string newName = baseFileName + "." + std::to_string(i + 1) + ".log";

            if (std::filesystem::exists(oldName)) {
                if (i == maxFileCount - 1) {
                    std::filesystem::remove(oldName);
                } else {
                    std::filesystem::rename(oldName, newName);
                }
            }
        }

        std::string currentName = baseFileName + ".log";
        std::string firstBackup = baseFileName + ".1.log";
        if (std::filesystem::exists(currentName)) {
            std::filesystem::rename(currentName, firstBackup);
        }

        openLogFile();
        currentSize = 0;
    }

    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

    std::ofstream logFile;
    std::string baseFileName;
    size_t maxSize;
    int maxFileCount;
    size_t currentSize;
};