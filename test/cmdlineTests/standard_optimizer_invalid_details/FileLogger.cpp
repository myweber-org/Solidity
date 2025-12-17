
#include <fstream>
#include <string>
#include <filesystem>
#include <chrono>
#include <iomanip>

namespace fs = std::filesystem;

class FileLogger {
private:
    std::ofstream logFile;
    std::string baseFilename;
    size_t maxFileSize;
    int maxBackupFiles;
    size_t currentSize;

    void rotateIfNeeded() {
        if (currentSize >= maxFileSize) {
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
            
            openLogFile();
        }
    }
    
    void openLogFile() {
        logFile.open(baseFilename, std::ios::app);
        currentSize = fs::file_size(baseFilename);
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
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        logFile << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] ";
        logFile << message << std::endl;
        
        currentSize = logFile.tellp();
        rotateIfNeeded();
    }
    
    void logWithLevel(const std::string& level, const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        logFile << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] ";
        logFile << "[" << level << "] " << message << std::endl;
        
        currentSize = logFile.tellp();
        rotateIfNeeded();
    }
};