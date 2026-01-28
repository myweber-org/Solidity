
#include <fstream>
#include <filesystem>
#include <chrono>
#include <zlib.h>
#include <string>
#include <vector>
#include <memory>

namespace fs = std::filesystem;

class FileLogger {
public:
    explicit FileLogger(const std::string& basePath, size_t maxSize = 1048576, int maxFiles = 10)
        : basePath_(basePath), maxSize_(maxSize), maxFiles_(maxFiles), currentSize_(0) {
        initializeLogFile();
    }

    void log(const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        char timestamp[64];
        std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&time));

        std::string entry = std::string(timestamp) + " - " + message + "\n";
        
        std::lock_guard<std::mutex> lock(mutex_);
        if (currentSize_ + entry.size() > maxSize_) {
            rotateLog();
        }
        
        fileStream_ << entry;
        fileStream_.flush();
        currentSize_ += entry.size();
    }

    void compressOldLogs() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& entry : fs::directory_iterator(fs::path(basePath_).parent_path())) {
            if (entry.path().extension() == ".log" && entry.path() != currentPath_) {
                compressFile(entry.path());
            }
        }
    }

private:
    void initializeLogFile() {
        fs::create_directories(fs::path(basePath_).parent_path());
        currentPath_ = generateNewFilename();
        fileStream_.open(currentPath_, std::ios::app);
        currentSize_ = fs::file_size(currentPath_);
    }

    std::string generateNewFilename() const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        char timestamp[64];
        std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", std::localtime(&time));
        return basePath_ + "_" + std::string(timestamp) + ".log";
    }

    void rotateLog() {
        fileStream_.close();
        compressFile(currentPath_);
        cleanupOldFiles();
        currentPath_ = generateNewFilename();
        fileStream_.open(currentPath_, std::ios::app);
        currentSize_ = 0;
    }

    void compressFile(const fs::path& sourcePath) {
        std::ifstream input(sourcePath, std::ios::binary);
        if (!input) return;

        std::vector<char> buffer(std::istreambuf_iterator<char>(input), {});
        input.close();

        fs::path destPath = sourcePath;
        destPath += ".gz";

        gzFile output = gzopen(destPath.string().c_str(), "wb");
        if (!output) return;

        gzwrite(output, buffer.data(), buffer.size());
        gzclose(output);

        fs::remove(sourcePath);
    }

    void cleanupOldFiles() {
        std::vector<fs::path> logFiles;
        for (const auto& entry : fs::directory_iterator(fs::path(basePath_).parent_path())) {
            if (entry.path().extension() == ".gz") {
                logFiles.push_back(entry.path());
            }
        }

        std::sort(logFiles.begin(), logFiles.end(), [](const fs::path& a, const fs::path& b) {
            return fs::last_write_time(a) < fs::last_write_time(b);
        });

        while (logFiles.size() > maxFiles_) {
            fs::remove(logFiles.front());
            logFiles.erase(logFiles.begin());
        }
    }

    std::string basePath_;
    size_t maxSize_;
    int maxFiles_;
    size_t currentSize_;
    fs::path currentPath_;
    std::ofstream fileStream_;
    std::mutex mutex_;
};