#include <fstream>
#include <mutex>
#include <string>
#include <vector>
#include <chrono>
#include <filesystem>
#include <zlib.h>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

class ThreadSafeFileLogger {
private:
    std::ofstream logFile;
    std::mutex fileMutex;
    std::string baseFilename;
    size_t maxFileSize;
    int maxBackupFiles;
    std::vector<std::string> backupFiles;

    void rotateIfNeeded() {
        std::lock_guard<std::mutex> lock(fileMutex);
        if (logFile.tellp() > static_cast<std::streampos>(maxFileSize)) {
            logFile.close();
            std::string timestamp = getCurrentTimestamp();
            std::string newFilename = baseFilename + "." + timestamp;
            fs::rename(baseFilename, newFilename);
            compressFile(newFilename);
            backupFiles.push_back(newFilename);
            manageBackups();
            logFile.open(baseFilename, std::ios::app);
        }
    }

    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");
        return ss.str();
    }

    void compressFile(const std::string& filename) {
        std::ifstream input(filename, std::ios::binary);
        if (!input) return;

        std::string compressedFilename = filename + ".gz";
        gzFile output = gzopen(compressedFilename.c_str(), "wb");
        if (!output) return;

        char buffer[1024];
        while (input.read(buffer, sizeof(buffer))) {
            gzwrite(output, buffer, input.gcount());
        }
        if (input.gcount() > 0) {
            gzwrite(output, buffer, input.gcount());
        }

        gzclose(output);
        fs::remove(filename);
    }

    void manageBackups() {
        while (backupFiles.size() > static_cast<size_t>(maxBackupFiles)) {
            fs::remove(backupFiles.front());
            backupFiles.erase(backupFiles.begin());
        }
    }

public:
    ThreadSafeFileLogger(const std::string& filename, size_t maxSize = 1048576, int maxBackups = 10)
        : baseFilename(filename), maxFileSize(maxSize), maxBackupFiles(maxBackups) {
        logFile.open(baseFilename, std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Cannot open log file: " + baseFilename);
        }
    }

    ~ThreadSafeFileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(fileMutex);
        logFile << getCurrentTimestamp() << " - " << message << std::endl;
        rotateIfNeeded();
    }

    void flush() {
        std::lock_guard<std::mutex> lock(fileMutex);
        logFile.flush();
    }

    std::vector<std::string> getBackupFiles() const {
        return backupFiles;
    }
};