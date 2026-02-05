#include <fstream>
#include <string>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

class FileLogger {
public:
    explicit FileLogger(const std::string& baseName, size_t maxFiles = 5, size_t maxSize = 1024 * 1024)
        : baseFilename(baseName), maxFileCount(maxFiles), maxFileSize(maxSize), currentSize(0) {
        openLogFile();
    }

    ~FileLogger() {
        if (logStream.is_open()) {
            logStream.close();
        }
    }

    void log(const std::string& message) {
        if (!logStream.is_open()) return;

        std::string entry = getCurrentTimestamp() + " - " + message + "\n";
        size_t entrySize = entry.size();

        if (currentSize + entrySize > maxFileSize) {
            rotateLogs();
        }

        logStream << entry;
        logStream.flush();
        currentSize += entrySize;
    }

private:
    std::string baseFilename;
    size_t maxFileCount;
    size_t maxFileSize;
    size_t currentSize;
    std::ofstream logStream;

    void openLogFile() {
        std::string filename = baseFilename + ".log";
        logStream.open(filename, std::ios::app);
        if (logStream.is_open()) {
            currentSize = fs::file_size(filename);
        }
    }

    void rotateLogs() {
        logStream.close();

        for (int i = maxFileCount - 1; i > 0; --i) {
            std::string oldName = baseFilename + "." + std::to_string(i) + ".log";
            std::string newName = baseFilename + "." + std::to_string(i + 1) + ".log";

            if (fs::exists(oldName)) {
                fs::rename(oldName, newName);
            }
        }

        std::string firstBackup = baseFilename + ".1.log";
        fs::rename(baseFilename + ".log", firstBackup);

        openLogFile();
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
};