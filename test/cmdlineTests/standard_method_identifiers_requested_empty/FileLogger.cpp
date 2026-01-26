
#include <fstream>
#include <string>
#include <chrono>
#include <filesystem>
#include <mutex>

class FileLogger {
private:
    std::ofstream logFile;
    std::string baseFilename;
    size_t maxFileSize;
    int maxBackupFiles;
    std::mutex writeMutex;
    size_t currentSize;

    void rotateIfNeeded() {
        if (currentSize >= maxFileSize) {
            logFile.close();
            
            for (int i = maxBackupFiles - 1; i > 0; --i) {
                std::string oldName = baseFilename + "." + std::to_string(i);
                std::string newName = baseFilename + "." + std::to_string(i + 1);
                
                if (std::filesystem::exists(oldName)) {
                    std::filesystem::rename(oldName, newName);
                }
            }
            
            std::string firstBackup = baseFilename + ".1";
            std::filesystem::rename(baseFilename, firstBackup);
            
            openLogFile();
        }
    }

    void openLogFile() {
        logFile.open(baseFilename, std::ios::app);
        currentSize = std::filesystem::file_size(baseFilename);
    }

public:
    FileLogger(const std::string& filename, size_t maxSize = 1048576, int backups = 5)
        : baseFilename(filename), maxFileSize(maxSize), maxBackupFiles(backups), currentSize(0) {
        openLogFile();
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(writeMutex);
        
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        char timestamp[20];
        std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
        
        std::string logEntry = "[" + std::string(timestamp) + "] " + message + "\n";
        
        rotateIfNeeded();
        
        logFile << logEntry;
        logFile.flush();
        currentSize += logEntry.size();
    }

    void logWithLevel(const std::string& level, const std::string& message) {
        log("[" + level + "] " + message);
    }
};