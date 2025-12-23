
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileLogger {
private:
    fs::path basePath;
    std::string baseName;
    size_t maxFiles;
    size_t maxFileSize;
    std::ofstream currentStream;
    size_t currentIndex;
    size_t currentSize;

    void rotateIfNeeded() {
        if (currentSize >= maxFileSize) {
            currentStream.close();
            currentIndex = (currentIndex + 1) % maxFiles;
            openCurrentFile();
        }
    }

    void openCurrentFile() {
        std::stringstream filename;
        filename << baseName << "_" << std::setfill('0') << std::setw(3) << currentIndex << ".log";
        fs::path fullPath = basePath / filename.str();

        currentStream.open(fullPath, std::ios::app);
        if (!currentStream.is_open()) {
            throw std::runtime_error("Cannot open log file: " + fullPath.string());
        }

        currentSize = fs::file_size(fullPath);
    }

    void cleanupOldFiles() {
        std::vector<fs::path> logFiles;
        for (const auto& entry : fs::directory_iterator(basePath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".log") {
                if (entry.path().stem().string().find(baseName) == 0) {
                    logFiles.push_back(entry.path());
                }
            }
        }

        std::sort(logFiles.begin(), logFiles.end(),
                  [](const fs::path& a, const fs::path& b) {
                      return fs::last_write_time(a) < fs::last_write_time(b);
                  });

        while (logFiles.size() > maxFiles) {
            fs::remove(logFiles.front());
            logFiles.erase(logFiles.begin());
        }
    }

public:
    FileLogger(const std::string& path, const std::string& name, size_t maxFiles = 10, size_t maxSize = 1024 * 1024)
        : basePath(path), baseName(name), maxFiles(maxFiles), maxFileSize(maxSize), currentIndex(0), currentSize(0) {
        
        if (!fs::exists(basePath)) {
            fs::create_directories(basePath);
        }

        cleanupOldFiles();
        openCurrentFile();
    }

    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }

    void log(const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::stringstream logEntry;
        logEntry << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        logEntry << '.' << std::setfill('0') << std::setw(3) << ms.count();
        logEntry << " - " << message << "\n";

        std::string entryStr = logEntry.str();
        currentStream << entryStr;
        currentStream.flush();

        currentSize += entryStr.size();
        rotateIfNeeded();
    }

    void flush() {
        if (currentStream.is_open()) {
            currentStream.flush();
        }
    }
};