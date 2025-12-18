
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

class FileLogger {
private:
    std::string logDir;
    std::string baseName;
    size_t maxFileSize;
    int maxBackupFiles;
    std::ofstream currentStream;
    size_t currentSize;
    std::string currentFilePath;

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
        return ss.str();
    }

    void rotateIfNeeded() {
        if (currentSize >= maxFileSize) {
            currentStream.close();
            
            for (int i = maxBackupFiles - 1; i > 0; --i) {
                std::string oldName = logDir + "/" + baseName + "." + std::to_string(i) + ".log";
                std::string newName = logDir + "/" + baseName + "." + std::to_string(i + 1) + ".log";
                
                if (fs::exists(oldName)) {
                    fs::rename(oldName, newName);
                }
            }
            
            std::string firstBackup = logDir + "/" + baseName + ".1.log";
            fs::rename(currentFilePath, firstBackup);
            
            openNewLogFile();
        }
    }

    void openNewLogFile() {
        std::string timestamp = getTimestamp();
        currentFilePath = logDir + "/" + baseName + "_" + timestamp + ".log";
        currentStream.open(currentFilePath, std::ios::app);
        currentSize = 0;
        
        if (!currentStream.is_open()) {
            throw std::runtime_error("Failed to open log file: " + currentFilePath);
        }
    }

public:
    FileLogger(const std::string& directory, const std::string& name, 
               size_t maxSize = 1048576, int maxBackups = 5)
        : logDir(directory), baseName(name), maxFileSize(maxSize), 
          maxBackupFiles(maxBackups), currentSize(0) {
        
        if (!fs::exists(logDir)) {
            fs::create_directories(logDir);
        }
        
        openNewLogFile();
    }

    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }

    void log(const std::string& message, const std::string& level = "INFO") {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream logEntry;
        logEntry << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        logEntry << " [" << level << "] " << message << std::endl;
        
        std::string logStr = logEntry.str();
        currentStream << logStr;
        currentStream.flush();
        
        currentSize += logStr.size();
        rotateIfNeeded();
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
        FileLogger logger("./logs", "application", 1024, 3);
        
        for (int i = 0; i < 100; ++i) {
            logger.info("Log entry number: " + std::to_string(i));
            logger.warning("This is a warning message");
            logger.error("This is an error message");
        }
        
        std::cout << "Logging completed. Check ./logs directory." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}