#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <string>
#include <iomanip>
#include <mutex>

namespace fs = std::filesystem;

class FileLogger {
private:
    std::ofstream logFile;
    std::string baseFilename;
    std::string logDirectory;
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

    void rotateLogIfNeeded() {
        if (!logFile.is_open()) return;

        logFile.seekp(0, std::ios::end);
        size_t currentSize = logFile.tellp();

        if (currentSize >= maxFileSize) {
            logFile.close();

            for (int i = maxBackupFiles - 1; i > 0; --i) {
                fs::path oldFile = logDirectory + "/" + baseFilename + "." + std::to_string(i);
                fs::path newFile = logDirectory + "/" + baseFilename + "." + std::to_string(i + 1);

                if (fs::exists(oldFile)) {
                    fs::rename(oldFile, newFile);
                }
            }

            fs::path currentLog = logDirectory + "/" + baseFilename;
            fs::path firstBackup = logDirectory + "/" + baseFilename + ".1";

            if (fs::exists(currentLog)) {
                fs::rename(currentLog, firstBackup);
            }

            openLogFile();
        }
    }

    void openLogFile() {
        fs::path fullPath = logDirectory + "/" + baseFilename;
        logFile.open(fullPath, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file: " << fullPath << std::endl;
        }
    }

public:
    FileLogger(const std::string& directory = "logs", 
               const std::string& filename = "app.log",
               size_t maxSize = 1048576, // 1 MB
               int backups = 5)
        : baseFilename(filename), logDirectory(directory), 
          maxFileSize(maxSize), maxBackupFiles(backups) {
        
        if (!fs::exists(logDirectory)) {
            fs::create_directories(logDirectory);
        }
        
        openLogFile();
        
        if (logFile.is_open()) {
            log("Logger initialized");
        }
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            log("Logger shutting down");
            logFile.close();
        }
    }

    void log(const std::string& message, const std::string& level = "INFO") {
        std::lock_guard<std::mutex> lock(logMutex);
        
        if (!logFile.is_open()) return;

        rotateLogIfNeeded();

        std::string timestamp = getCurrentTimestamp();
        logFile << "[" << timestamp << "] [" << level << "] " << message << std::endl;
        logFile.flush();
    }

    void error(const std::string& message) {
        log(message, "ERROR");
    }

    void warning(const std::string& message) {
        log(message, "WARNING");
    }

    void info(const std::string& message) {
        log(message, "INFO");
    }

    void debug(const std::string& message) {
        log(message, "DEBUG");
    }
};

void exampleUsage() {
    FileLogger logger("logs", "application.log", 1024, 3); // 1KB max size for testing
    
    for (int i = 0; i < 100; ++i) {
        logger.info("Log entry number: " + std::to_string(i));
        logger.debug("Debug information for iteration " + std::to_string(i));
        
        if (i % 10 == 0) {
            logger.warning("Reached iteration multiple of 10: " + std::to_string(i));
        }
        
        if (i == 50) {
            logger.error("Simulated error at iteration 50");
        }
    }
}

int main() {
    exampleUsage();
    return 0;
}