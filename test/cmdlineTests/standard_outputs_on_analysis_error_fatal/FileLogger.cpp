
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <zlib.h>
#include <string>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

class FileLogger {
private:
    std::ofstream logFile;
    std::string baseName;
    size_t maxSize;
    int maxFiles;
    size_t currentSize;
    
    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
        return ss.str();
    }
    
    void rotateIfNeeded() {
        if (currentSize >= maxSize) {
            logFile.close();
            std::string oldName = baseName + ".log";
            std::string newName = baseName + "_" + getTimestamp() + ".log";
            
            if (fs::exists(oldName)) {
                fs::rename(oldName, newName);
                compressFile(newName);
                cleanupOldFiles();
            }
            
            logFile.open(oldName, std::ios::app);
            currentSize = 0;
        }
    }
    
    void compressFile(const std::string& filename) {
        std::ifstream inFile(filename, std::ios::binary);
        if (!inFile) return;
        
        std::string compressedName = filename + ".gz";
        gzFile outFile = gzopen(compressedName.c_str(), "wb");
        if (!outFile) return;
        
        char buffer[8192];
        while (inFile.read(buffer, sizeof(buffer)) || inFile.gcount()) {
            gzwrite(outFile, buffer, inFile.gcount());
        }
        
        gzclose(outFile);
        inFile.close();
        fs::remove(filename);
    }
    
    void cleanupOldFiles() {
        std::vector<fs::directory_entry> files;
        for (const auto& entry : fs::directory_iterator(".")) {
            if (entry.path().extension() == ".gz" && 
                entry.path().string().find(baseName) != std::string::npos) {
                files.push_back(entry);
            }
        }
        
        std::sort(files.begin(), files.end(), 
                 [](const fs::directory_entry& a, const fs::directory_entry& b) {
                     return fs::last_write_time(a) < fs::last_write_time(b);
                 });
        
        while (files.size() > static_cast<size_t>(maxFiles)) {
            fs::remove(files.front().path());
            files.erase(files.begin());
        }
    }
    
public:
    FileLogger(const std::string& name, size_t sizeMB = 10, int keepFiles = 5) 
        : baseName(name), maxSize(sizeMB * 1024 * 1024), maxFiles(keepFiles), currentSize(0) {
        logFile.open(baseName + ".log", std::ios::app);
    }
    
    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }
    
    void log(const std::string& message, const std::string& level = "INFO") {
        std::lock_guard<std::mutex> lock(logMutex);
        std::string timestamp = getTimestamp();
        std::string logEntry = "[" + timestamp + "] [" + level + "] " + message + "\n";
        
        rotateIfNeeded();
        
        logFile << logEntry;
        logFile.flush();
        currentSize += logEntry.size();
    }
    
    void error(const std::string& message) {
        log(message, "ERROR");
    }
    
    void warning(const std::string& message) {
        log(message, "WARNING");
    }
    
    void debug(const std::string& message) {
        log(message, "DEBUG");
    }
    
private:
    std::mutex logMutex;
};