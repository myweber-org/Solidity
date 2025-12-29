
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <zlib.h>
#include <vector>

namespace fs = std::filesystem;

class FileLogger {
private:
    fs::path logDir;
    std::string baseName;
    size_t maxFileSize;
    int maxBackupCount;
    std::ofstream currentStream;
    size_t currentSize;

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
        return ss.str();
    }

    void compressFile(const fs::path& source) {
        std::ifstream inFile(source, std::ios::binary);
        if (!inFile) return;

        std::vector<char> buffer(std::istreambuf_iterator<char>(inFile), {});
        inFile.close();

        uLongf compressedSize = compressBound(buffer.size());
        std::vector<Bytef> compressed(compressedSize);

        if (compress(compressed.data(), &compressedSize,
                    reinterpret_cast<Bytef*>(buffer.data()), buffer.size()) == Z_OK) {
            fs::path dest = source;
            dest += ".gz";
            std::ofstream outFile(dest, std::ios::binary);
            outFile.write(reinterpret_cast<char*>(compressed.data()), compressedSize);
            outFile.close();
            fs::remove(source);
        }
    }

    void rotateIfNeeded() {
        if (currentSize >= maxFileSize) {
            currentStream.close();
            fs::path oldPath = logDir / (baseName + ".log");
            fs::path newPath = logDir / (baseName + "_" + getTimestamp() + ".log");
            fs::rename(oldPath, newPath);
            compressFile(newPath);
            cleanupOldBackups();
            openNewLog();
        }
    }

    void cleanupOldBackups() {
        std::vector<fs::path> backups;
        for (const auto& entry : fs::directory_iterator(logDir)) {
            if (entry.path().filename().string().find(baseName) != std::string::npos &&
                entry.path().extension() == ".gz") {
                backups.push_back(entry.path());
            }
        }

        std::sort(backups.begin(), backups.end(),
                 [](const fs::path& a, const fs::path& b) {
                     return fs::last_write_time(a) < fs::last_write_time(b);
                 });

        while (backups.size() > static_cast<size_t>(maxBackupCount)) {
            fs::remove(backups.front());
            backups.erase(backups.begin());
        }
    }

    void openNewLog() {
        fs::path logPath = logDir / (baseName + ".log");
        currentStream.open(logPath, std::ios::app);
        currentSize = fs::file_size(logPath);
    }

public:
    FileLogger(const std::string& dir, const std::string& name,
               size_t maxSize = 1048576, int backups = 5)
        : logDir(dir), baseName(name), maxFileSize(maxSize),
          maxBackupCount(backups), currentSize(0) {
        if (!fs::exists(logDir)) {
            fs::create_directories(logDir);
        }
        openNewLog();
    }

    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "[%Y-%m-%d %H:%M:%S] ");
        ss << message << std::endl;

        std::string logEntry = ss.str();
        currentStream << logEntry;
        currentStream.flush();
        currentSize += logEntry.size();

        rotateIfNeeded();
    }

    void logError(const std::string& message) {
        log("[ERROR] " + message);
    }

    void logWarning(const std::string& message) {
        log("[WARNING] " + message);
    }

    void logInfo(const std::string& message) {
        log("[INFO] " + message);
    }

private:
    std::mutex logMutex;
};