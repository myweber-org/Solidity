
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <mutex>

namespace fs = std::filesystem;

class FileLogger {
private:
    std::ofstream logFile;
    std::string baseFilename;
    size_t maxFileSize;
    int maxBackupFiles;
    std::mutex logMutex;

    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    void rotateIfNeeded() {
        if (logFile.tellp() > maxFileSize) {
            logFile.close();
            
            for (int i = maxBackupFiles - 1; i > 0; --i) {
                fs::path oldFile = baseFilename + "." + std::to_string(i);
                fs::path newFile = baseFilename + "." + std::to_string(i + 1);
                
                if (fs::exists(oldFile)) {
                    fs::rename(oldFile, newFile);
                }
            }
            
            fs::path firstBackup = baseFilename + ".1";
            if (fs::exists(baseFilename)) {
                fs::rename(baseFilename, firstBackup);
            }
            
            logFile.open(baseFilename, std::ios::app);
        }
    }

public:
    FileLogger(const std::string& filename, size_t maxSize = 1048576, int backups = 5)
        : baseFilename(filename), maxFileSize(maxSize), maxBackupFiles(backups) {
        logFile.open(filename, std::ios::app);
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(const std::string& message, const std::string& level = "INFO") {
        std::lock_guard<std::mutex> lock(logMutex);
        
        if (!logFile.is_open()) {
            return;
        }
        
        rotateIfNeeded();
        
        logFile << "[" << getCurrentTimestamp() << "] "
                << "[" << level << "] "
                << message << std::endl;
        
        logFile.flush();
    }

    void info(const std::string& message) {
        log(message, "INFO");
    }

    void warning(const std::string& message) {
        log(message, "WARNING");
    }

    void error(const std::string& message) {
        log(message, "ERROR");
    }
};

void exampleUsage() {
    FileLogger logger("application.log", 1024 * 1024, 3);
    
    logger.info("Application started");
    logger.warning("Low disk space detected");
    logger.error("Failed to connect to database");
    
    for (int i = 0; i < 1000; ++i) {
        logger.info("Processing item " + std::to_string(i));
    }
}

int main() {
    exampleUsage();
    return 0;
}