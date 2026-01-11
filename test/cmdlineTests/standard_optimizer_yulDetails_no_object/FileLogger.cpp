
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
    std::string baseName;
    size_t maxFileSize;
    int maxBackupCount;
    bool compressOld;

    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
        return ss.str();
    }

    void rotateIfNeeded(const fs::path& currentPath) {
        if (!fs::exists(currentPath)) return;

        if (fs::file_size(currentPath) < maxFileSize) return;

        std::string timestamp = getCurrentTimestamp();
        fs::path backupPath = logDirectory / (baseName + "_" + timestamp + ".log");

        if (compressOld) {
            backupPath.replace_extension(".log.gz");
            compressFile(currentPath, backupPath);
            fs::remove(currentPath);
        } else {
            fs::rename(currentPath, backupPath);
        }

        cleanupOldBackups();
    }

    void compressFile(const fs::path& source, const fs::path& dest) {
        std::ifstream inFile(source, std::ios::binary);
        if (!inFile) return;

        gzFile outFile = gzopen(dest.string().c_str(), "wb");
        if (!outFile) return;

        std::vector<char> buffer(8192);
        while (inFile.read(buffer.data(), buffer.size()) || inFile.gcount()) {
            gzwrite(outFile, buffer.data(), inFile.gcount());
        }

        gzclose(outFile);
    }

    void cleanupOldBackups() {
        std::vector<fs::path> backups;
        for (const auto& entry : fs::directory_iterator(logDirectory)) {
            if (entry.path().filename().string().find(baseName) != std::string::npos &&
                entry.path().extension() != ".log") {
                backups.push_back(entry.path());
            }
        }

        std::sort(backups.begin(), backups.end(),
                  [](const fs::path& a, const fs::path& b) {
                      return fs::last_write_time(a) > fs::last_write_time(b);
                  });

        while (backups.size() > static_cast<size_t>(maxBackupCount)) {
            fs::remove(backups.back());
            backups.pop_back();
        }
    }

public:
    FileLogger(const std::string& dir, const std::string& name,
               size_t maxSize = 1048576, int backups = 10, bool compress = true)
        : logDirectory(dir), baseName(name), maxFileSize(maxSize),
          maxBackupCount(backups), compressOld(compress) {
        if (!fs::exists(logDirectory)) {
            fs::create_directories(logDirectory);
        }
    }

    void log(const std::string& message) {
        fs::path currentLog = logDirectory / (baseName + ".log");
        rotateIfNeeded(currentLog);

        std::ofstream outFile(currentLog, std::ios::app);
        if (!outFile) return;

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        outFile << std::put_time(std::localtime(&time), "[%Y-%m-%d %H:%M:%S] ")
                << message << std::endl;
    }

    void logWithLevel(const std::string& level, const std::string& message) {
        log("[" + level + "] " + message);
    }
};