
#include <fstream>
#include <filesystem>
#include <chrono>
#include <zlib.h>
#include <string>
#include <vector>
#include <mutex>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

class FileLogger {
private:
    std::string logDir;
    std::string baseName;
    size_t maxFileSize;
    int maxBackupCount;
    bool compressBackups;
    std::mutex logMutex;
    std::ofstream currentStream;
    size_t currentSize;

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
        return ss.str();
    }

    std::string getCurrentFileName() {
        return logDir + "/" + baseName + ".log";
    }

    std::string getBackupFileName(int index) {
        std::stringstream ss;
        ss << logDir << "/" << baseName << "_" << getTimestamp();
        if (index > 0) {
            ss << "_" << index;
        }
        ss << ".log";
        if (compressBackups) {
            ss << ".gz";
        }
        return ss.str();
    }

    void rotateIfNeeded() {
        if (currentSize >= maxFileSize) {
            currentStream.close();
            for (int i = maxBackupCount - 1; i > 0; --i) {
                std::string oldName = getBackupFileName(i - 1);
                std::string newName = getBackupFileName(i);
                if (fs::exists(oldName)) {
                    fs::rename(oldName, newName);
                }
            }
            std::string firstBackup = getBackupFileName(0);
            fs::rename(getCurrentFileName(), firstBackup);
            if (compressBackups) {
                compressFile(firstBackup);
            }
            openCurrentFile();
        }
    }

    void compressFile(const std::string& filename) {
        std::ifstream inFile(filename, std::ios::binary);
        if (!inFile) return;

        std::vector<char> buffer(std::istreambuf_iterator<char>(inFile), {});
        inFile.close();

        std::string gzFilename = filename + ".gz";
        gzFile outFile = gzopen(gzFilename.c_str(), "wb");
        if (!outFile) return;

        gzwrite(outFile, buffer.data(), buffer.size());
        gzclose(outFile);
        fs::remove(filename);
    }

    void openCurrentFile() {
        currentStream.open(getCurrentFileName(), std::ios::app);
        currentSize = fs::file_size(getCurrentFileName());
    }

public:
    FileLogger(const std::string& dir, const std::string& name, size_t maxSize = 1048576, int backups = 5, bool compress = true)
        : logDir(dir), baseName(name), maxFileSize(maxSize), maxBackupCount(backups), compressBackups(compress), currentSize(0) {
        if (!fs::exists(logDir)) {
            fs::create_directories(logDir);
        }
        openCurrentFile();
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
};