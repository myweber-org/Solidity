
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
    fs::path logDirectory;
    std::string baseFilename;
    size_t maxFileSize;
    int maxBackupFiles;
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
            std::string timestamp = getTimestamp();
            fs::path oldPath = logDirectory / (baseFilename + ".log");
            fs::path newPath = logDirectory / (baseFilename + "_" + timestamp + ".log.gz");
            
            compressFile(oldPath, newPath);
            fs::remove(oldPath);
            
            cleanupOldBackups();
            
            currentStream.open(oldPath, std::ios::app);
            currentSize = 0;
        }
    }

    void compressFile(const fs::path& source, const fs::path& destination) {
        std::ifstream inFile(source, std::ios::binary);
        if (!inFile) return;

        gzFile outFile = gzopen(destination.string().c_str(), "wb");
        if (!outFile) return;

        std::vector<char> buffer(8192);
        while (inFile.read(buffer.data(), buffer.size()) || inFile.gcount()) {
            gzwrite(outFile, buffer.data(), inFile.gcount());
        }

        gzclose(outFile);
    }

    void cleanupOldBackups() {
        std::vector<fs::path> backups;
        for (const auto& entry : fs::directory_iterator(logDirectory)) {
            if (entry.path().extension() == ".gz" && 
                entry.path().string().find(baseFilename) != std::string::npos) {
                backups.push_back(entry.path());
            }
        }

        std::sort(backups.begin(), backups.end(), 
                  [](const fs::path& a, const fs::path& b) {
                      return fs::last_write_time(a) < fs::last_write_time(b);
                  });

        while (backups.size() > static_cast<size_t>(maxBackupFiles)) {
            fs::remove(backups.front());
            backups.erase(backups.begin());
        }
    }

public:
    FileLogger(const std::string& directory, const std::string& filename, 
               size_t maxSize = 1048576, int maxBackups = 10)
        : logDirectory(directory), baseFilename(filename), 
          maxFileSize(maxSize), maxBackupFiles(maxBackups), currentSize(0) {
        
        if (!fs::exists(logDirectory)) {
            fs::create_directories(logDirectory);
        }

        fs::path logPath = logDirectory / (baseFilename + ".log");
        currentStream.open(logPath, std::ios::app);
    }

    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        std::string timestamp = getTimestamp();
        std::string logEntry = "[" + timestamp + "] " + message + "\n";
        
        currentStream << logEntry;
        currentStream.flush();
        
        currentSize += logEntry.size();
        rotateIfNeeded();
    }

    void logWithLevel(const std::string& level, const std::string& message) {
        log("[" + level + "] " + message);
    }

private:
    std::mutex logMutex;
};