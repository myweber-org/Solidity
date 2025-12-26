
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

class FileLogger {
private:
    std::string logDirectory;
    std::string baseFilename;
    size_t maxFileSize;
    int maxBackupFiles;

    std::string generateTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
        return ss.str();
    }

    void rotateLogs() {
        std::string currentPath = logDirectory + "/" + baseFilename + ".log";
        if (fs::exists(currentPath)) {
            fs::file_size(currentPath);
            if (fs::file_size(currentPath) >= maxFileSize) {
                std::string backupPath = logDirectory + "/" + baseFilename + "_" + generateTimestamp() + ".log";
                fs::rename(currentPath, backupPath);
                cleanupOldBackups();
            }
        }
    }

    void cleanupOldBackups() {
        std::vector<fs::path> backupFiles;
        for (const auto& entry : fs::directory_iterator(logDirectory)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename.find(baseFilename + "_") == 0 && filename.find(".log") != std::string::npos) {
                    backupFiles.push_back(entry.path());
                }
            }
        }
        std::sort(backupFiles.begin(), backupFiles.end());
        while (backupFiles.size() > static_cast<size_t>(maxBackupFiles)) {
            fs::remove(backupFiles.front());
            backupFiles.erase(backupFiles.begin());
        }
    }

public:
    FileLogger(const std::string& dir, const std::string& baseName, size_t maxSize = 1048576, int maxBackups = 5)
        : logDirectory(dir), baseFilename(baseName), maxFileSize(maxSize), maxBackupFiles(maxBackups) {
        if (!fs::exists(logDirectory)) {
            fs::create_directories(logDirectory);
        }
    }

    void log(const std::string& message, const std::string& level = "INFO") {
        rotateLogs();
        std::string logPath = logDirectory + "/" + baseFilename + ".log";
        std::ofstream logFile(logPath, std::ios::app);
        if (logFile.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            logFile << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
            logFile << " [" << level << "] " << message << std::endl;
            logFile.close();
        } else {
            std::cerr << "Failed to open log file: " << logPath << std::endl;
        }
    }

    void info(const std::string& message) {
        log(message, "INFO");
    }

    void warning(const std::string& message) {
        log(message, "WARNING");
    }

    void error(const std::string& message) {
        log(message, "ERROR");
    }
};

int main() {
    FileLogger logger("./logs", "application", 1024 * 1024, 3);
    logger.info("Application started successfully.");
    logger.warning("Disk usage is above 80%.");
    logger.error("Failed to connect to database.");
    logger.info("Application shutdown initiated.");
    return 0;
}