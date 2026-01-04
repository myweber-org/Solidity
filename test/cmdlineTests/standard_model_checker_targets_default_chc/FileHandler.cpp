#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

class ThreadSafeLogger {
private:
    std::ofstream logFile;
    std::mutex logMutex;
    std::string baseFilename;
    size_t maxFileSize;
    int maxBackupFiles;
    size_t currentSize;

    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

    void rotateLogIfNeeded() {
        if (currentSize >= maxFileSize) {
            logFile.close();

            for (int i = maxBackupFiles - 1; i > 0; --i) {
                fs::path oldFile = baseFilename + "." + std::to_string(i);
                fs::path newFile = baseFilename + "." + std::to_string(i + 1);
                if (fs::exists(oldFile)) {
                    fs::rename(oldFile, newFile);
                }
            }

            fs::path currentLog = baseFilename;
            fs::path firstBackup = baseFilename + ".1";
            if (fs::exists(currentLog)) {
                fs::rename(currentLog, firstBackup);
            }

            openLogFile();
            currentSize = 0;
        }
    }

    void openLogFile() {
        logFile.open(baseFilename, std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Failed to open log file: " + baseFilename);
        }
        currentSize = static_cast<size_t>(fs::file_size(baseFilename));
    }

public:
    ThreadSafeLogger(const std::string& filename, size_t maxSize = 1048576, int maxBackups = 5)
        : baseFilename(filename), maxFileSize(maxSize), maxBackupFiles(maxBackups), currentSize(0) {
        openLogFile();
    }

    ~ThreadSafeLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(const std::string& message, const std::string& level = "INFO") {
        std::lock_guard<std::mutex> lock(logMutex);
        rotateLogIfNeeded();

        std::string entry = "[" + getCurrentTimestamp() + "] [" + level + "] " + message + "\n";
        logFile << entry;
        logFile.flush();
        currentSize += entry.size();
    }

    void debug(const std::string& message) { log(message, "DEBUG"); }
    void info(const std::string& message) { log(message, "INFO"); }
    void warning(const std::string& message) { log(message, "WARNING"); }
    void error(const std::string& message) { log(message, "ERROR"); }
};

void exampleUsage() {
    ThreadSafeLogger logger("application.log", 1024, 3);

    logger.info("Application started");
    logger.debug("Initializing components");
    logger.warning("Resource usage is high");
    logger.error("Failed to connect to database");

    for (int i = 0; i < 100; ++i) {
        logger.info("Processing item " + std::to_string(i));
    }
}

int main() {
    try {
        exampleUsage();
        std::cout << "Logging completed. Check application.log files." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}