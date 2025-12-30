#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

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
        if (!fs::exists(currentPath)) {
            return;
        }

        if (fs::file_size(currentPath) < maxFileSize) {
            return;
        }

        for (int i = maxBackupFiles - 1; i > 0; --i) {
            std::string oldName = logDirectory + "/" + baseFilename + ".log." + std::to_string(i);
            std::string newName = logDirectory + "/" + baseFilename + ".log." + std::to_string(i + 1);
            if (fs::exists(oldName)) {
                fs::rename(oldName, newName);
            }
        }

        std::string firstBackup = logDirectory + "/" + baseFilename + ".log.1";
        fs::rename(currentPath, firstBackup);
    }

public:
    FileLogger(const std::string& dir, const std::string& filename, size_t maxSize = 1048576, int backups = 5)
        : logDirectory(dir), baseFilename(filename), maxFileSize(maxSize), maxBackupFiles(backups) {
        if (!fs::exists(logDirectory)) {
            fs::create_directories(logDirectory);
        }
    }

    void log(const std::string& message, const std::string& level = "INFO") {
        rotateLogs();

        std::string logPath = logDirectory + "/" + baseFilename + ".log";
        std::ofstream logFile(logPath, std::ios::app);

        if (logFile.is_open()) {
            std::string timestamp = generateTimestamp();
            logFile << "[" << timestamp << "] [" << level << "] " << message << std::endl;
            logFile.close();
        } else {
            std::cerr << "Failed to open log file: " << logPath << std::endl;
        }
    }

    void debug(const std::string& message) {
        log(message, "DEBUG");
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
    FileLogger logger("./logs", "application");
    
    logger.info("Application started");
    logger.debug("Initializing components");
    logger.warning("Resource usage is high");
    logger.error("Failed to connect to database");
    logger.info("Application shutdown");

    return 0;
}