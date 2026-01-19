#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

class FileLogger {
private:
    std::string baseName;
    std::string logDir;
    size_t maxFileSize;
    int maxFiles;
    std::ofstream currentFile;
    size_t currentSize;

    std::string generateTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
        return ss.str();
    }

    std::string constructFileName(int index = 0) {
        std::stringstream ss;
        ss << logDir << "/" << baseName;
        if (index > 0) {
            ss << "_" << index;
        }
        ss << ".log";
        return ss.str();
    }

    void rotateFiles() {
        for (int i = maxFiles - 1; i > 0; --i) {
            std::string oldName = constructFileName(i);
            std::string newName = constructFileName(i + 1);
            if (fs::exists(oldName)) {
                fs::rename(oldName, newName);
            }
        }
        std::string firstBackup = constructFileName(1);
        if (fs::exists(firstBackup)) {
            fs::remove(firstBackup);
        }
        std::string current = constructFileName();
        if (fs::exists(current)) {
            fs::rename(current, firstBackup);
        }
    }

    void openNewFile() {
        std::string fileName = constructFileName();
        currentFile.open(fileName, std::ios::app);
        if (!currentFile.is_open()) {
            throw std::runtime_error("Cannot open log file: " + fileName);
        }
        currentSize = fs::file_size(fileName);
    }

public:
    FileLogger(const std::string& name = "app", const std::string& dir = "logs",
               size_t maxSize = 1048576, int maxCount = 5)
        : baseName(name), logDir(dir), maxFileSize(maxSize), maxFiles(maxCount), currentSize(0) {
        if (!fs::exists(logDir)) {
            fs::create_directories(logDir);
        }
        openNewFile();
    }

    ~FileLogger() {
        if (currentFile.is_open()) {
            currentFile.close();
        }
    }

    void log(const std::string& message) {
        std::string entry = "[" + generateTimestamp() + "] " + message + "\n";
        size_t entrySize = entry.size();

        if (currentSize + entrySize > maxFileSize) {
            currentFile.close();
            rotateFiles();
            openNewFile();
        }

        currentFile << entry;
        currentFile.flush();
        currentSize += entrySize;
    }

    void logInfo(const std::string& message) {
        log("[INFO] " + message);
    }

    void logWarning(const std::string& message) {
        log("[WARNING] " + message);
    }

    void logError(const std::string& message) {
        log("[ERROR] " + message);
    }
};

int main() {
    try {
        FileLogger logger("myapp", "logs", 1024, 3);

        for (int i = 0; i < 50; ++i) {
            logger.logInfo("This is log entry number " + std::to_string(i));
            logger.logWarning("Sample warning message");
            logger.logError("Sample error message");
        }

        std::cout << "Logging completed. Check 'logs' directory." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Logging error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}