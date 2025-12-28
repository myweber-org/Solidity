
#include <fstream>
#include <string>
#include <filesystem>
#include <chrono>
#include <zlib.h>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

class FileLogger {
private:
    std::string logDir;
    std::string baseName;
    size_t maxFileSize;
    int maxBackupCount;
    std::ofstream currentStream;
    size_t currentSize;
    
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
            std::string oldPath = logDir + "/" + baseName + ".log";
            std::string newPath = logDir + "/" + baseName + "_" + getTimestamp() + ".log.gz";
            
            compressFile(oldPath, newPath);
            fs::remove(oldPath);
            
            currentStream.open(oldPath, std::ios::app);
            currentSize = 0;
            cleanupOldBackups();
        }
    }
    
    void compressFile(const std::string& source, const std::string& dest) {
        std::ifstream inFile(source, std::ios::binary);
        gzFile outFile = gzopen(dest.c_str(), "wb");
        
        char buffer[1024];
        while (inFile.read(buffer, sizeof(buffer))) {
            gzwrite(outFile, buffer, inFile.gcount());
        }
        gzclose(outFile);
        inFile.close();
    }
    
    void cleanupOldBackups() {
        std::vector<fs::path> backups;
        for (const auto& entry : fs::directory_iterator(logDir)) {
            if (entry.path().extension() == ".gz" && 
                entry.path().string().find(baseName) != std::string::npos) {
                backups.push_back(entry.path());
            }
        }
        
        std::sort(backups.begin(), backups.end());
        while (backups.size() > static_cast<size_t>(maxBackupCount)) {
            fs::remove(backups.front());
            backups.erase(backups.begin());
        }
    }
    
public:
    FileLogger(const std::string& dir, const std::string& name, 
               size_t maxSize = 1048576, int backups = 10) 
        : logDir(dir), baseName(name), maxFileSize(maxSize), 
          maxBackupCount(backups), currentSize(0) {
        
        fs::create_directories(logDir);
        std::string filePath = logDir + "/" + baseName + ".log";
        currentStream.open(filePath, std::ios::app);
    }
    
    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }
    
    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        std::string entry = "[" + getTimestamp() + "] " + message + "\n";
        currentStream << entry;
        currentStream.flush();
        currentSize += entry.size();
        rotateIfNeeded();
    }
    
    void logWithLevel(const std::string& level, const std::string& message) {
        log(level + ": " + message);
    }
    
private:
    std::mutex logMutex;
};