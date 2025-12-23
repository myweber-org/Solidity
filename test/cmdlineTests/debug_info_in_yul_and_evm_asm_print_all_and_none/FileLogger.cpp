
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <string>
#include <zlib.h>
#include <iomanip>
#include <sstream>

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
            fs::rename(oldName, newName);
            compressFile(newName);
            cleanupOldFiles();
            openLogFile();
        }
    }

    void compressFile(const std::string& filename) {
        std::ifstream inFile(filename, std::ios::binary);
        if (!inFile) return;

        std::string compressedName = filename + ".gz";
        gzFile outFile = gzopen(compressedName.c_str(), "wb");
        if (!outFile) {
            inFile.close();
            return;
        }

        char buffer[1024];
        while (inFile.read(buffer, sizeof(buffer)) || inFile.gcount()) {
            gzwrite(outFile, buffer, inFile.gcount());
        }

        gzclose(outFile);
        inFile.close();
        fs::remove(filename);
    }

    void cleanupOldFiles() {
        std::vector<fs::path> logFiles;
        for (const auto& entry : fs::directory_iterator(fs::current_path())) {
            if (entry.path().extension() == ".gz" && 
                entry.path().string().find(baseName) != std::string::npos) {
                logFiles.push_back(entry.path());
            }
        }

        std::sort(logFiles.begin(), logFiles.end(), 
                  [](const fs::path& a, const fs::path& b) {
                      return fs::last_write_time(a) < fs::last_write_time(b);
                  });

        while (logFiles.size() > static_cast<size_t>(maxFiles)) {
            fs::remove(logFiles.front());
            logFiles.erase(logFiles.begin());
        }
    }

    void openLogFile() {
        std::string logFileName = baseName + ".log";
        logFile.open(logFileName, std::ios::app);
        currentSize = fs::file_size(logFileName);
    }

public:
    FileLogger(const std::string& name, size_t sizeMB = 10, int fileCount = 5) 
        : baseName(name), maxSize(sizeMB * 1024 * 1024), maxFiles(fileCount) {
        openLogFile();
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(const std::string& message) {
        rotateIfNeeded();
        
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "[%Y-%m-%d %H:%M:%S] ");
        
        logFile << ss.str() << message << std::endl;
        currentSize += message.length() + ss.str().length() + 1;
    }

    void logError(const std::string& error) {
        log("[ERROR] " + error);
    }

    void logWarning(const std::string& warning) {
        log("[WARNING] " + warning);
    }

    void logInfo(const std::string& info) {
        log("[INFO] " + info);
    }
};

int main() {
    FileLogger logger("application", 1, 3);
    
    for (int i = 0; i < 100; ++i) {
        logger.logInfo("Processing item " + std::to_string(i));
        if (i % 10 == 0) {
            logger.logWarning("Checkpoint reached at iteration " + std::to_string(i));
        }
        if (i == 50) {
            logger.logError("Simulated error at midpoint");
        }
    }
    
    return 0;
}