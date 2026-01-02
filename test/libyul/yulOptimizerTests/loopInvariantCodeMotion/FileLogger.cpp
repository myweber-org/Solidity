
#include <fstream>
#include <filesystem>
#include <chrono>
#include <sstream>
#include <zlib.h>
#include <vector>
#include <iomanip>

namespace fs = std::filesystem;

class FileLogger {
private:
    fs::path logDirectory;
    std::string logPrefix;
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

    void compressFile(const fs::path& source) {
        std::ifstream input(source, std::ios::binary);
        if (!input) return;

        std::vector<char> buffer(std::istreambuf_iterator<char>(input), {});
        input.close();

        fs::path dest = source;
        dest += ".gz";

        gzFile gz = gzopen(dest.string().c_str(), "wb");
        if (gz) {
            gzwrite(gz, buffer.data(), buffer.size());
            gzclose(gz);
            fs::remove(source);
        }
    }

    void rotateLogs() {
        if (currentStream.is_open()) {
            currentStream.close();
        }

        std::vector<fs::path> existingLogs;
        for (const auto& entry : fs::directory_iterator(logDirectory)) {
            if (entry.path().extension() == ".log" || entry.path().extension() == ".gz") {
                existingLogs.push_back(entry.path());
            }
        }

        std::sort(existingLogs.begin(), existingLogs.end());
        while (existingLogs.size() >= maxBackupFiles) {
            fs::remove(existingLogs.front());
            existingLogs.erase(existingLogs.begin());
        }

        for (auto& log : existingLogs) {
            if (log.extension() == ".log") {
                compressFile(log);
            }
        }
    }

    void openNewLog() {
        std::string filename = logPrefix + "_" + getTimestamp() + ".log";
        fs::path fullPath = logDirectory / filename;
        currentStream.open(fullPath, std::ios::app);
        currentSize = 0;
    }

public:
    FileLogger(const std::string& directory, const std::string& prefix = "app",
               size_t maxSize = 1048576, int backups = 5)
        : logDirectory(directory), logPrefix(prefix),
          maxFileSize(maxSize), maxBackupFiles(backups), currentSize(0) {
        
        if (!fs::exists(logDirectory)) {
            fs::create_directories(logDirectory);
        }
        rotateLogs();
        openNewLog();
    }

    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }

    void log(const std::string& message) {
        if (!currentStream.is_open()) return;

        std::string entry = "[" + getTimestamp() + "] " + message + "\n";
        currentStream << entry;
        currentStream.flush();
        currentSize += entry.size();

        if (currentSize >= maxFileSize) {
            currentStream.close();
            rotateLogs();
            openNewLog();
        }
    }

    void logWithLevel(const std::string& level, const std::string& message) {
        log(level + ": " + message);
    }
};