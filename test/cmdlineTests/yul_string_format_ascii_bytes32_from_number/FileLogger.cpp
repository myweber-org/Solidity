
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <zlib.h>
#include <vector>
#include <memory>

namespace fs = std::filesystem;

class FileLogger {
private:
    std::ofstream logFile;
    std::string basePath;
    size_t maxFileSize;
    int maxBackupCount;
    bool compressBackups;

    void rotateIfNeeded() {
        if (!logFile.is_open()) return;
        
        logFile.flush();
        auto currentPos = logFile.tellp();
        if (currentPos >= static_cast<std::streampos>(maxFileSize)) {
            logFile.close();
            rotateFiles();
            openCurrentLog();
        }
    }

    void rotateFiles() {
        for (int i = maxBackupCount - 1; i > 0; --i) {
            fs::path oldFile = basePath + "." + std::to_string(i);
            if (compressBackups && fs::exists(oldFile)) {
                oldFile += ".gz";
            }
            
            fs::path newFile = basePath + "." + std::to_string(i + 1);
            if (compressBackups) {
                newFile += ".gz";
            }
            
            if (fs::exists(oldFile)) {
                fs::rename(oldFile, newFile);
            }
        }

        fs::path firstBackup = basePath + ".1";
        if (fs::exists(basePath)) {
            if (compressBackups) {
                compressFile(basePath, firstBackup.string() + ".gz");
                fs::remove(basePath);
            } else {
                fs::rename(basePath, firstBackup);
            }
        }
    }

    void compressFile(const std::string& source, const std::string& dest) {
        std::ifstream inFile(source, std::ios::binary);
        if (!inFile) return;

        gzFile outFile = gzopen(dest.c_str(), "wb");
        if (!outFile) return;

        std::vector<char> buffer(8192);
        while (inFile.read(buffer.data(), buffer.size()) || inFile.gcount()) {
            gzwrite(outFile, buffer.data(), inFile.gcount());
        }

        gzclose(outFile);
    }

    void openCurrentLog() {
        logFile.open(basePath, std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Cannot open log file: " + basePath);
        }
    }

public:
    FileLogger(const std::string& path, size_t maxSize = 10485760, int backups = 5, bool compress = true)
        : basePath(path), maxFileSize(maxSize), maxBackupCount(backups), compressBackups(compress) {
        openCurrentLog();
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        logFile << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        logFile << '.' << std::setfill('0') << std::setw(3) << ms.count();
        logFile << " - " << message << std::endl;

        rotateIfNeeded();
    }

    void flush() {
        if (logFile.is_open()) {
            logFile.flush();
        }
    }
};