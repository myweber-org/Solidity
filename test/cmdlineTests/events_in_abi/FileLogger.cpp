
#include <fstream>
#include <string>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

class FileLogger {
private:
    std::string logDirectory;
    std::string baseFilename;
    size_t maxFileSize;
    int maxBackupFiles;
    std::ofstream currentStream;
    std::string currentFilePath;
    
    std::string generateTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
        return ss.str();
    }
    
    void rotateIfNeeded() {
        if (!currentStream.is_open()) return;
        
        currentStream.seekp(0, std::ios::end);
        size_t currentSize = currentStream.tellp();
        
        if (currentSize >= maxFileSize) {
            currentStream.close();
            
            for (int i = maxBackupFiles - 1; i > 0; --i) {
                std::string oldName = logDirectory + "/" + baseFilename + "." + std::to_string(i);
                std::string newName = logDirectory + "/" + baseFilename + "." + std::to_string(i + 1);
                
                if (fs::exists(oldName)) {
                    fs::rename(oldName, newName);
                }
            }
            
            std::string firstBackup = logDirectory + "/" + baseFilename + ".1";
            fs::rename(currentFilePath, firstBackup);
            
            openNewLogFile();
        }
    }
    
    void openNewLogFile() {
        std::string timestamp = generateTimestamp();
        currentFilePath = logDirectory + "/" + baseFilename + "_" + timestamp + ".log";
        currentStream.open(currentFilePath, std::ios::app);
        
        if (!currentStream) {
            throw std::runtime_error("Cannot open log file: " + currentFilePath);
        }
    }
    
public:
    FileLogger(const std::string& dir, const std::string& filename, 
               size_t maxSize = 1048576, int maxBackups = 5)
        : logDirectory(dir), baseFilename(filename), 
          maxFileSize(maxSize), maxBackupFiles(maxBackups) {
        
        if (!fs::exists(logDirectory)) {
            fs::create_directories(logDirectory);
        }
        
        openNewLogFile();
    }
    
    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }
    
    void log(const std::string& message) {
        rotateIfNeeded();
        
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        currentStream << std::put_time(std::localtime(&time), "[%Y-%m-%d %H:%M:%S] ")
                     << message << std::endl;
    }
    
    void logWithLevel(const std::string& level, const std::string& message) {
        rotateIfNeeded();
        
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        currentStream << std::put_time(std::localtime(&time), "[%Y-%m-%d %H:%M:%S] ")
                     << "[" << level << "] " << message << std::endl;
    }
};