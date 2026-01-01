
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
    fs::path logDir;
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

    void compressFile(const fs::path& source) {
        std::ifstream inFile(source, std::ios::binary);
        if (!inFile) return;

        std::vector<char> buffer(std::istreambuf_iterator<char>(inFile), {});
        inFile.close();

        uLongf compressedSize = compressBound(buffer.size());
        std::vector<Bytef> compressed(compressedSize);

        if (compress(compressed.data(), &compressedSize,
                     reinterpret_cast<Bytef*>(buffer.data()), buffer.size()) == Z_OK) {
            fs::path dest = source;
            dest += ".gz";
            std::ofstream outFile(dest, std::ios::binary);
            outFile.write(reinterpret_cast<char*>(compressed.data()), compressedSize);
            outFile.close();
            fs::remove(source);
        }
    }

    void rotateIfNeeded() {
        if (currentSize >= maxFileSize) {
            currentStream.close();
            fs::path oldPath = logDir / (baseName + ".log");
            fs::path newPath = logDir / (baseName + "_" + getTimestamp() + ".log");
            if (fs::exists(oldPath)) {
                fs::rename(oldPath, newPath);
                compressFile(newPath);
            }

            std::vector<fs::path> backups;
            for (const auto& entry : fs::directory_iterator(logDir)) {
                if (entry.path().filename().string().find(baseName) != std::string::npos &&
                    entry.path().extension() == ".gz") {
                    backups.push_back(entry.path());
                }
            }

            std::sort(backups.begin(), backups.end());
            while (backups.size() > static_cast<size_t>(maxBackupCount)) {
                fs::remove(backups.front());
                backups.erase(backups.begin());
            }

            currentStream.open(oldPath, std::ios::app);
            currentSize = 0;
        }
    }

public:
    FileLogger(const std::string& directory, const std::string& name,
               size_t maxSize = 1048576, int backups = 5)
        : logDir(directory), baseName(name), maxFileSize(maxSize),
          maxBackupCount(backups), currentSize(0) {
        if (!fs::exists(logDir)) {
            fs::create_directories(logDir);
        }

        fs::path logFile = logDir / (baseName + ".log");
        currentStream.open(logFile, std::ios::app);
        if (currentStream) {
            currentStream.seekp(0, std::ios::end);
            currentSize = currentStream.tellp();
        }
    }

    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }

    void log(const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "[%Y-%m-%d %H:%M:%S] ");
        ss << message << std::endl;

        std::string logEntry = ss.str();
        rotateIfNeeded();

        if (currentStream) {
            currentStream << logEntry;
            currentStream.flush();
            currentSize += logEntry.size();
        }
    }
};

int main() {
    FileLogger logger("logs", "application", 1024, 3);
    
    for (int i = 0; i < 100; ++i) {
        logger.log("Test log entry number: " + std::to_string(i));
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    return 0;
}