
#include <fstream>
#include <filesystem>
#include <chrono>
#include <sstream>
#include <zlib.h>
#include <iomanip>

namespace fs = std::filesystem;

class FileLogger {
private:
    std::string logDir;
    std::string baseName;
    size_t maxFileSize;
    int maxBackupCount;
    bool compressBackups;
    
    std::ofstream currentStream;
    std::string currentFilePath;
    size_t currentSize;
    
    std::string generateTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
        ss << "_" << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }
    
    void rotateIfNeeded() {
        if (currentSize >= maxFileSize) {
            closeCurrent();
            rotateFiles();
            openNew();
        }
    }
    
    void rotateFiles() {
        for (int i = maxBackupCount - 1; i >= 0; --i) {
            std::string oldName, newName;
            
            if (i == 0) {
                oldName = currentFilePath;
            } else {
                oldName = logDir + "/" + baseName + "_" + 
                         std::to_string(i - 1) + ".log";
            }
            
            if (fs::exists(oldName)) {
                newName = logDir + "/" + baseName + "_" + 
                         std::to_string(i) + ".log";
                
                if (i == maxBackupCount - 1) {
                    fs::remove(oldName);
                } else {
                    fs::rename(oldName, newName);
                    
                    if (compressBackups && i > 0) {
                        compressFile(newName);
                    }
                }
            }
        }
    }
    
    void compressFile(const std::string& path) {
        std::ifstream inFile(path, std::ios::binary);
        if (!inFile) return;
        
        std::string compressedPath = path + ".gz";
        gzFile outFile = gzopen(compressedPath.c_str(), "wb");
        if (!outFile) return;
        
        char buffer[8192];
        while (inFile.read(buffer, sizeof(buffer)) || inFile.gcount()) {
            gzwrite(outFile, buffer, inFile.gcount());
        }
        
        gzclose(outFile);
        fs::remove(path);
    }
    
    void closeCurrent() {
        if (currentStream.is_open()) {
            currentStream.flush();
            currentStream.close();
        }
    }
    
    void openNew() {
        std::string timestamp = generateTimestamp();
        currentFilePath = logDir + "/" + baseName + "_" + timestamp + ".log";
        currentStream.open(currentFilePath, std::ios::app);
        currentSize = 0;
    }
    
public:
    FileLogger(const std::string& directory, 
               const std::string& name = "app",
               size_t maxSize = 10485760, // 10MB
               int backups = 5,
               bool compress = true)
        : logDir(directory), baseName(name), maxFileSize(maxSize),
          maxBackupCount(backups), compressBackups(compress), currentSize(0) {
        
        if (!fs::exists(logDir)) {
            fs::create_directories(logDir);
        }
        
        openNew();
    }
    
    ~FileLogger() {
        closeCurrent();
    }
    
    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(writeMutex);
        
        if (!currentStream.is_open()) {
            openNew();
        }
        
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        currentStream << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        currentStream << "." << std::setfill('0') << std::setw(3) << ms.count();
        currentStream << " " << message << std::endl;
        
        currentSize = currentStream.tellp();
        rotateIfNeeded();
    }
    
    void flush() {
        std::lock_guard<std::mutex> lock(writeMutex);
        if (currentStream.is_open()) {
            currentStream.flush();
        }
    }
    
private:
    std::mutex writeMutex;
};