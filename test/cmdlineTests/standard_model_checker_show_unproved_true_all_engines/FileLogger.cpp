
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
    int maxBackupFiles;
    std::ofstream currentStream;
    std::string currentFilePath;
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
            std::string newPath = logDir + "/" + baseName + "_" + timestamp + ".log";
            fs::rename(currentFilePath, newPath);
            compressFile(newPath);
            openNewFile();
        }
    }

    void compressFile(const std::string& path) {
        std::ifstream inFile(path, std::ios::binary);
        if (!inFile) return;

        std::vector<char> buffer(std::istreambuf_iterator<char>(inFile), {});
        inFile.close();

        std::string compressedPath = path + ".gz";
        gzFile outFile = gzopen(compressedPath.c_str(), "wb");
        if (!outFile) return;

        gzwrite(outFile, buffer.data(), buffer.size());
        gzclose(outFile);

        fs::remove(path);
        cleanupOldBackups();
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
        while (backups.size() > static_cast<size_t>(maxBackupFiles)) {
            fs::remove(backups.front());
            backups.erase(backups.begin());
        }
    }

    void openNewFile() {
        currentFilePath = logDir + "/" + baseName + ".log";
        currentStream.open(currentFilePath, std::ios::app);
        currentSize = fs::file_size(currentFilePath);
    }

public:
    FileLogger(const std::string& dir, const std::string& name, size_t maxSize = 1048576, int maxBackups = 10)
        : logDir(dir), baseName(name), maxFileSize(maxSize), maxBackupFiles(maxBackups), currentSize(0) {
        
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
        std::lock_guard<std::mutex> lock(logMutex);
        std::string timestamp = getTimestamp();
        std::string logEntry = "[" + timestamp + "] " + message + "\n";
        
        currentStream << logEntry;
        currentStream.flush();
        currentSize += logEntry.size();
        
        rotateIfNeeded();
    }

private:
    std::mutex logMutex;
};

int main() {
    FileLogger logger("./logs", "app", 1024, 5);
    
    for (int i = 0; i < 100; ++i) {
        logger.log("Test log entry number: " + std::to_string(i));
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    return 0;
}