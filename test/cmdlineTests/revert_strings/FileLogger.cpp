
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <zlib.h>
#include <vector>

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
    
    std::string getCurrentFilename() {
        return logDir + "/" + baseName + ".log";
    }
    
    void rotateIfNeeded() {
        if (currentSize >= maxFileSize) {
            currentStream.close();
            std::string oldFile = getCurrentFilename();
            std::string newFile = logDir + "/" + baseName + "_" + getTimestamp() + ".log";
            fs::rename(oldFile, newFile);
            compressFile(newFile);
            cleanupOldBackups();
            openNewFile();
        }
    }
    
    void compressFile(const std::string& filename) {
        std::ifstream inFile(filename, std::ios::binary);
        if (!inFile) return;
        
        std::vector<char> buffer(
            (std::istreambuf_iterator<char>(inFile)),
            std::istreambuf_iterator<char>()
        );
        inFile.close();
        
        uLongf compressedSize = compressBound(buffer.size());
        std::vector<Bytef> compressed(compressedSize);
        
        if (compress(compressed.data(), &compressedSize, 
                    reinterpret_cast<Bytef*>(buffer.data()), buffer.size()) == Z_OK) {
            std::ofstream outFile(filename + ".gz", std::ios::binary);
            outFile.write(reinterpret_cast<char*>(compressed.data()), compressedSize);
            outFile.close();
            fs::remove(filename);
        }
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
        while (backups.size() > maxBackupCount) {
            fs::remove(backups.front());
            backups.erase(backups.begin());
        }
    }
    
    void openNewFile() {
        std::string filename = getCurrentFilename();
        currentStream.open(filename, std::ios::app);
        currentSize = fs::file_size(filename);
    }
    
public:
    FileLogger(const std::string& dir, const std::string& name, 
               size_t maxSize = 1048576, int maxBackups = 10)
        : logDir(dir), baseName(name), maxFileSize(maxSize), maxBackupCount(maxBackups), currentSize(0) {
        if (!fs::exists(logDir)) {
            fs::create_directories(logDir);
        }
        openNewFile();
    }
    
    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }
    
    void log(const std::string& message) {
        std::string timestamp = getTimestamp();
        std::string logEntry = "[" + timestamp + "] " + message + "\n";
        
        currentStream << logEntry;
        currentStream.flush();
        currentSize += logEntry.size();
        
        rotateIfNeeded();
    }
    
    void logWithLevel(const std::string& level, const std::string& message) {
        log(level + ": " + message);
    }
};

int main() {
    FileLogger logger("./logs", "application", 1024, 5);
    
    for (int i = 0; i < 100; ++i) {
        logger.logWithLevel("INFO", "Processing item " + std::to_string(i));
        logger.logWithLevel("DEBUG", "Detailed information about item " + std::to_string(i));
    }
    
    return 0;
}