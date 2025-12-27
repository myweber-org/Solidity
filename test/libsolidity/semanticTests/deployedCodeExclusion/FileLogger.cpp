
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
            fs::path oldPath = logDir / (baseName + ".log");
            fs::path newPath = logDir / (baseName + "_" + timestamp + ".log");

            if (fs::exists(oldPath)) {
                fs::rename(oldPath, newPath);
                compressFile(newPath);
                cleanupOldFiles();
            }

            currentStream.open(oldPath, std::ios::app);
            currentSize = 0;
        }
    }

    void compressFile(const fs::path& filePath) {
        std::ifstream inFile(filePath, std::ios::binary);
        if (!inFile) return;

        std::vector<char> buffer(std::istreambuf_iterator<char>(inFile), {});
        inFile.close();

        fs::path compressedPath = filePath;
        compressedPath += ".gz";

        gzFile outFile = gzopen(compressedPath.string().c_str(), "wb");
        if (!outFile) return;

        gzwrite(outFile, buffer.data(), buffer.size());
        gzclose(outFile);

        fs::remove(filePath);
    }

    void cleanupOldFiles() {
        std::vector<fs::path> backupFiles;
        for (const auto& entry : fs::directory_iterator(logDir)) {
            if (entry.path().filename().string().find(baseName) != std::string::npos &&
                entry.path().extension() == ".gz") {
                backupFiles.push_back(entry.path());
            }
        }

        std::sort(backupFiles.begin(), backupFiles.end(),
                  [](const fs::path& a, const fs::path& b) {
                      return fs::last_write_time(a) < fs::last_write_time(b);
                  });

        while (backupFiles.size() > static_cast<size_t>(maxBackupFiles)) {
            fs::remove(backupFiles.front());
            backupFiles.erase(backupFiles.begin());
        }
    }

public:
    FileLogger(const std::string& directory, const std::string& name,
               size_t maxSize = 1048576, int maxBackups = 10)
        : logDir(directory), baseName(name), maxFileSize(maxSize),
          maxBackupFiles(maxBackups), currentSize(0) {

        if (!fs::exists(logDir)) {
            fs::create_directories(logDir);
        }

        fs::path logFile = logDir / (baseName + ".log");
        currentStream.open(logFile, std::ios::app);
        if (currentStream) {
            currentSize = fs::file_size(logFile);
        }
    }

    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        rotateIfNeeded();

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "[%Y-%m-%d %H:%M:%S] ");
        ss << message << std::endl;

        std::string logEntry = ss.str();
        currentStream << logEntry;
        currentStream.flush();
        currentSize += logEntry.size();
    }

    FileLogger(const FileLogger&) = delete;
    FileLogger& operator=(const FileLogger&) = delete;

private:
    std::mutex logMutex;
};