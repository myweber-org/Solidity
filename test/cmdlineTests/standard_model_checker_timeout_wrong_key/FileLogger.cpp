
#include <fstream>
#include <string>
#include <filesystem>
#include <chrono>
#include <iomanip>

namespace fs = std::filesystem;

class FileLogger {
private:
    std::ofstream logFile;
    std::string baseName;
    size_t maxSize;
    int maxFiles;
    size_t currentSize;

    void rotateIfNeeded() {
        if (currentSize >= maxSize) {
            logFile.close();
            
            for (int i = maxFiles - 1; i > 0; --i) {
                std::string oldName = baseName + "." + std::to_string(i);
                std::string newName = baseName + "." + std::to_string(i + 1);
                
                if (fs::exists(oldName)) {
                    fs::rename(oldName, newName);
                }
            }
            
            std::string firstBackup = baseName + ".1";
            fs::rename(baseName, firstBackup);
            
            openLogFile();
        }
    }

    void openLogFile() {
        logFile.open(baseName, std::ios::app);
        currentSize = fs::file_size(baseName);
    }

public:
    FileLogger(const std::string& filename, size_t maxFileSize = 1048576, int backupCount = 5)
        : baseName(filename), maxSize(maxFileSize), maxFiles(backupCount), currentSize(0) {
        
        if (!fs::exists(baseName)) {
            std::ofstream createFile(baseName);
            createFile.close();
        }
        
        openLogFile();
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        logFile << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] "
                << message << std::endl;
        
        currentSize = logFile.tellp();
        rotateIfNeeded();
    }

    void log(const std::string& level, const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        logFile << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] "
                << "[" << level << "] " << message << std::endl;
        
        currentSize = logFile.tellp();
        rotateIfNeeded();
    }
};