
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

class FileLogger {
private:
    std::ofstream logFile;
    std::string basePath;
    size_t maxFileSize;
    int maxBackupFiles;

    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d_%H-%M-%S");
        return ss.str();
    }

    void rotateIfNeeded() {
        if (!logFile.is_open()) return;

        logFile.seekp(0, std::ios::end);
        size_t currentSize = logFile.tellp();

        if (currentSize >= maxFileSize) {
            logFile.close();
            
            for (int i = maxBackupFiles - 1; i > 0; --i) {
                fs::path oldFile = basePath + ".backup" + std::to_string(i);
                fs::path newFile = basePath + ".backup" + std::to_string(i + 1);
                
                if (fs::exists(oldFile)) {
                    if (fs::exists(newFile)) {
                        fs::remove(newFile);
                    }
                    fs::rename(oldFile, newFile);
                }
            }

            fs::path firstBackup = basePath + ".backup1";
            if (fs::exists(firstBackup)) {
                fs::remove(firstBackup);
            }
            fs::rename(basePath, firstBackup);

            openLogFile();
        }
    }

    void openLogFile() {
        logFile.open(basePath, std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Cannot open log file: " + basePath);
        }
    }

public:
    FileLogger(const std::string& path, size_t maxSize = 1048576, int maxBackups = 5)
        : basePath(path), maxFileSize(maxSize), maxBackupFiles(maxBackups) {
        openLogFile();
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(const std::string& message, const std::string& level = "INFO") {
        rotateIfNeeded();
        
        std::string timestamp = getCurrentTimestamp();
        logFile << "[" << timestamp << "] [" << level << "] " << message << std::endl;
        logFile.flush();
    }

    void debug(const std::string& message) {
        log(message, "DEBUG");
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

int main() {
    try {
        FileLogger logger("application.log", 1024, 3);
        
        for (int i = 0; i < 100; ++i) {
            logger.info("Log entry number: " + std::to_string(i));
            logger.debug("Debug information for iteration " + std::to_string(i));
            
            if (i % 10 == 0) {
                logger.warning("Reached iteration multiple of 10: " + std::to_string(i));
            }
        }
        
        logger.error("Simulated error occurred at the end of the test");
        
        std::cout << "Logging completed. Check application.log and backup files." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Logging error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}