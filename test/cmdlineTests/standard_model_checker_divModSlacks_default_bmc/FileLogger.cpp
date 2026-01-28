#include <fstream>
#include <string>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

class FileLogger {
public:
    explicit FileLogger(const std::string& baseName, size_t maxSize = 1048576, int maxFiles = 5)
        : baseFilename(baseName), maxFileSize(maxSize), maxBackupFiles(maxFiles), currentSize(0) {
        openLogFile();
    }

    ~FileLogger() {
        if (logStream.is_open()) {
            logStream.close();
        }
    }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        if (!logStream.is_open()) {
            return;
        }

        std::string entry = getCurrentTimestamp() + " - " + message + "\n";
        logStream << entry;
        logStream.flush();
        currentSize += entry.size();

        if (currentSize >= maxFileSize) {
            rotateLogFiles();
        }
    }

private:
    std::string baseFilename;
    size_t maxFileSize;
    int maxBackupFiles;
    std::ofstream logStream;
    size_t currentSize;
    std::mutex logMutex;

    void openLogFile() {
        logStream.open(baseFilename, std::ios::app);
        if (logStream.is_open()) {
            currentSize = fs::file_size(baseFilename);
        }
    }

    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

    void rotateLogFiles() {
        logStream.close();

        for (int i = maxBackupFiles - 1; i > 0; --i) {
            std::string oldName = baseFilename + "." + std::to_string(i);
            std::string newName = baseFilename + "." + std::to_string(i + 1);

            if (fs::exists(oldName)) {
                if (fs::exists(newName)) {
                    fs::remove(newName);
                }
                fs::rename(oldName, newName);
            }
        }

        std::string firstBackup = baseFilename + ".1";
        if (fs::exists(firstBackup)) {
            fs::remove(firstBackup);
        }
        fs::rename(baseFilename, firstBackup);

        openLogFile();
        currentSize = 0;
    }
};