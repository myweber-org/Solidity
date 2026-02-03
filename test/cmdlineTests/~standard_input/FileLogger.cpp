
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
    std::string basePath;
    size_t maxFileSize;
    int maxBackupCount;
    int currentBackup;

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
        return ss.str();
    }

    void rotateIfNeeded() {
        if (logFile.tellp() > maxFileSize) {
            logFile.close();
            std::string newName = basePath + "." + getTimestamp() + ".log";
            fs::rename(basePath, newName);
            compressFile(newName);
            cleanupOldBackups();
            logFile.open(basePath, std::ios::app);
            currentBackup++;
        }
    }

    void compressFile(const std::string& filename) {
        std::ifstream inFile(filename, std::ios::binary);
        if (!inFile) return;

        std::string compressedName = filename + ".gz";
        gzFile outFile = gzopen(compressedName.c_str(), "wb");
        if (!outFile) return;

        char buffer[1024];
        while (inFile.read(buffer, sizeof(buffer))) {
            gzwrite(outFile, buffer, inFile.gcount());
        }
        gzwrite(outFile, buffer, inFile.gcount());
        gzclose(outFile);
        fs::remove(filename);
    }

    void cleanupOldBackups() {
        std::vector<fs::path> backups;
        for (const auto& entry : fs::directory_iterator(fs::path(basePath).parent_path())) {
            if (entry.path().string().find(basePath + ".") != std::string::npos) {
                backups.push_back(entry.path());
            }
        }
        std::sort(backups.begin(), backups.end());
        while (backups.size() > maxBackupCount) {
            fs::remove(backups.front());
            backups.erase(backups.begin());
        }
    }

public:
    FileLogger(const std::string& path, size_t maxSize = 1048576, int backups = 5)
        : basePath(path), maxFileSize(maxSize), maxBackupCount(backups), currentBackup(0) {
        logFile.open(path, std::ios::app);
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(const std::string& message) {
        rotateIfNeeded();
        logFile << getTimestamp() << " - " << message << std::endl;
    }

    void logWithLevel(const std::string& level, const std::string& message) {
        rotateIfNeeded();
        logFile << getTimestamp() << " [" << level << "] " << message << std::endl;
    }
};