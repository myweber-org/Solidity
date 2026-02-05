
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
    fs::path basePath;
    size_t maxFileSize;
    int maxBackupFiles;
    std::ofstream currentStream;
    std::string currentFilename;
    
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
            compressCurrentFile();
            cleanupOldFiles();
            openNewFile();
        }
    }
    
    void compressCurrentFile() {
        if (!fs::exists(currentFilename)) return;
        
        std::ifstream inFile(currentFilename, std::ios::binary);
        if (!inFile) return;
        
        std::vector<char> buffer(
            (std::istreambuf_iterator<char>(inFile)),
            std::istreambuf_iterator<char>()
        );
        inFile.close();
        
        std::string compressedFilename = currentFilename + ".gz";
        gzFile outFile = gzopen(compressedFilename.c_str(), "wb");
        if (!outFile) return;
        
        gzwrite(outFile, buffer.data(), buffer.size());
        gzclose(outFile);
        
        fs::remove(currentFilename);
    }
    
    void cleanupOldFiles() {
        std::vector<fs::path> backupFiles;
        
        for (const auto& entry : fs::directory_iterator(basePath)) {
            if (entry.path().extension() == ".gz") {
                backupFiles.push_back(entry.path());
            }
        }
        
        std::sort(backupFiles.begin(), backupFiles.end());
        
        while (backupFiles.size() > static_cast<size_t>(maxBackupFiles)) {
            fs::remove(backupFiles.front());
            backupFiles.erase(backupFiles.begin());
        }
    }
    
    void openNewFile() {
        currentFilename = (basePath / ("log_" + generateTimestamp() + ".txt")).string();
        currentStream.open(currentFilename, std::ios::app);
    }
    
public:
    FileLogger(const std::string& path = "./logs", 
               size_t maxSize = 1048576, 
               int maxBackups = 10)
        : basePath(path), maxFileSize(maxSize), maxBackupFiles(maxBackups) {
        
        if (!fs::exists(basePath)) {
            fs::create_directories(basePath);
        }
        
        openNewFile();
    }
    
    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }
    
    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        
        if (!currentStream.is_open()) {
            openNewFile();
        }
        
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        currentStream << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
                     << "] " << message << std::endl;
        
        rotateIfNeeded();
    }
    
    void logWithLevel(const std::string& level, const std::string& message) {
        log("[" + level + "] " + message);
    }
    
private:
    std::mutex logMutex;
};