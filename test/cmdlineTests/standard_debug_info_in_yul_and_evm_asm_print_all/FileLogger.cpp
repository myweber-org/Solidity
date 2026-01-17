
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
            cleanupOldFiles();
            
            currentStream.open(oldPath, std::ios::app);
            currentSize = 0;
        }
    }

    void compressFile(const fs::path& source, const fs::path& destination) {
        std::ifstream input(source, std::ios::binary);
        if (!input) return;

        gzFile output = gzopen(destination.string().c_str(), "wb");
        if (!output) {
            input.close();
            return;
        }

        std::vector<char> buffer(8192);
        while (input.read(buffer.data(), buffer.size()) || input.gcount()) {
            gzwrite(output, buffer.data(), input.gcount());
        }

        input.close();
        gzclose(output);
        fs::remove(source);
    }

    void cleanupOldFiles() {
        std::vector<fs::path> backupFiles;
        for (const auto& entry : fs::directory_iterator(logDirectory)) {
            if (entry.path().extension() == ".gz" && 
                entry.path().stem().string().find(baseFilename) == 0) {
                backupFiles.push_back(entry.path());
            }
        }

        std::sort(backupFiles.begin(), backupFiles.end());
        while (backupFiles.size() > maxBackupFiles) {
            fs::remove(backupFiles.front());
            backupFiles.erase(backupFiles.begin());
        }
    }

public:
    FileLogger(const std::string& directory, const std::string& filename, 
               size_t maxSize = 10485760, int maxBackups = 10)
        : logDirectory(directory), baseFilename(filename), 
          maxFileSize(maxSize), maxBackupFiles(maxBackups), currentSize(0) {
        
        if (!fs::exists(logDirectory)) {
            fs::create_directories(logDirectory);
        }

        fs::path logPath = logDirectory / (baseFilename + ".log");
        currentStream.open(logPath, std::ios::app);
        if (currentStream) {
            currentSize = fs::file_size(logPath);
        }
    }

    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        if (!currentStream.is_open()) return;

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