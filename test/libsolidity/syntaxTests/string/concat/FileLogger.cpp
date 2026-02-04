#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <string>
#include <zlib.h>
#include <vector>

namespace fs = std::filesystem;

class FileLogger {
private:
    fs::path logDirectory;
    std::string baseFilename;
    size_t maxFileSize;
    int maxBackupFiles;
    std::ofstream currentStream;
    size_t currentSize;

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", std::localtime(&time));
        return std::string(buffer);
    }

    void rotateLog() {
        if (currentStream.is_open()) {
            currentStream.close();
        }

        std::string timestamp = getTimestamp();
        fs::path oldPath = logDirectory / (baseFilename + ".log");
        fs::path newPath = logDirectory / (baseFilename + "_" + timestamp + ".log.gz");

        if (fs::exists(oldPath)) {
            compressFile(oldPath, newPath);
            fs::remove(oldPath);
        }

        currentStream.open(oldPath, std::ios::app);
        currentSize = 0;
        cleanupOldBackups();
    }

    void compressFile(const fs::path& source, const fs::path& dest) {
        std::ifstream inFile(source, std::ios::binary);
        if (!inFile) return;

        gzFile outFile = gzopen(dest.string().c_str(), "wb");
        if (!outFile) return;

        std::vector<char> buffer(8192);
        while (inFile.read(buffer.data(), buffer.size()) || inFile.gcount()) {
            gzwrite(outFile, buffer.data(), inFile.gcount());
        }

        gzclose(outFile);
    }

    void cleanupOldBackups() {
        std::vector<fs::path> backups;
        for (const auto& entry : fs::directory_iterator(logDirectory)) {
            if (entry.path().extension() == ".gz" &&
                entry.path().string().find(baseFilename) != std::string::npos) {
                backups.push_back(entry.path());
            }
        }

        std::sort(backups.begin(), backups.end());
        while (backups.size() > static_cast<size_t>(maxBackupFiles)) {
            fs::remove(backups.front());
            backups.erase(backups.begin());
        }
    }

public:
    FileLogger(const std::string& dir, const std::string& filename,
               size_t maxSize = 1048576, int backups = 10)
        : logDirectory(dir), baseFilename(filename),
          maxFileSize(maxSize), maxBackupFiles(backups), currentSize(0) {
        
        if (!fs::exists(logDirectory)) {
            fs::create_directories(logDirectory);
        }

        fs::path currentLog = logDirectory / (baseFilename + ".log");
        if (fs::exists(currentLog)) {
            currentSize = fs::file_size(currentLog);
        }

        currentStream.open(currentLog, std::ios::app);
    }

    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }

    void log(const std::string& message) {
        std::string entry = "[" + getTimestamp() + "] " + message + "\n";
        
        if (currentSize + entry.size() > maxFileSize) {
            rotateLog();
        }

        currentStream << entry;
        currentStream.flush();
        currentSize += entry.size();
    }

    void logError(const std::string& error) {
        log("ERROR: " + error);
    }

    void logWarning(const std::string& warning) {
        log("WARNING: " + warning);
    }

    void logInfo(const std::string& info) {
        log("INFO: " + info);
    }
};

int main() {
    FileLogger logger("logs", "application", 102400, 5);
    
    logger.logInfo("Application started");
    logger.logWarning("Configuration file not found, using defaults");
    
    for (int i = 0; i < 100; ++i) {
        logger.log("Processing item " + std::to_string(i));
    }
    
    logger.logError("Failed to connect to database");
    logger.logInfo("Application shutting down");
    
    return 0;
}