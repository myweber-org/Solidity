
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <filesystem>

class FileLogger {
private:
    std::ofstream logFile;
    std::mutex logMutex;
    std::string baseName;
    size_t maxSize;
    int maxFiles;
    
    void rotateIfNeeded() {
        if (logFile.tellp() > maxSize) {
            logFile.close();
            
            for (int i = maxFiles - 1; i > 0; --i) {
                std::string oldName = baseName + "." + std::to_string(i);
                std::string newName = baseName + "." + std::to_string(i + 1);
                
                if (std::filesystem::exists(oldName)) {
                    std::filesystem::rename(oldName, newName);
                }
            }
            
            std::filesystem::rename(baseName, baseName + ".1");
            logFile.open(baseName, std::ios::app);
        }
    }
    
    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

public:
    FileLogger(const std::string& filename, size_t maxFileSize = 1048576, int keepFiles = 5)
        : baseName(filename), maxSize(maxFileSize), maxFiles(keepFiles) {
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
        
        logFile << "[" << getTimestamp() << "] "
                << "[" << level << "] "
                << message << std::endl;
        
        logFile.flush();
    }
    
    void info(const std::string& message) {
        log(message, "INFO");
    }
    
    void warning(const std::string& message) {
        log(message, "WARN");
    }
    
    void error(const std::string& message) {
        log(message, "ERROR");
    }
};