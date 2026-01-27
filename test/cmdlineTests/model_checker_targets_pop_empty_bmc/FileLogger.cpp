
#include <fstream>
#include <iostream>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

class FileLogger {
private:
    std::string baseFilename;
    std::string logDirectory;
    size_t maxFileSize;
    int maxBackupFiles;

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::tm tm_now = *std::localtime(&time_t_now);

        std::ostringstream oss;
        oss << std::put_time(&tm_now, "%Y%m%d_%H%M%S");
        return oss.str();
    }

    void rotateLogs() {
        std::string currentLogPath = logDirectory + "/" + baseFilename + ".log";

        if (fs::exists(currentLogPath)) {
            fs::file_size(currentLogPath);
            if (fs::file_size(currentLogPath) >= maxFileSize) {
                for (int i = maxBackupFiles - 1; i > 0; --i) {
                    std::string oldName = logDirectory + "/" + baseFilename + ".log." + std::to_string(i);
                    std::string newName = logDirectory + "/" + baseFilename + ".log." + std::to_string(i + 1);
                    if (fs::exists(oldName)) {
                        fs::rename(oldName, newName);
                    }
                }
                std::string firstBackup = logDirectory + "/" + baseFilename + ".log.1";
                fs::rename(currentLogPath, firstBackup);
            }
        }
    }

public:
    FileLogger(const std::string& filename, const std::string& dir = "logs", size_t maxSize = 1048576, int maxBackups = 5)
        : baseFilename(filename), logDirectory(dir), maxFileSize(maxSize), maxBackupFiles(maxBackups) {
        if (!fs::exists(logDirectory)) {
            fs::create_directories(logDirectory);
        }
    }

    void log(const std::string& message, const std::string& level = "INFO") {
        rotateLogs();

        std::string logPath = logDirectory + "/" + baseFilename + ".log";
        std::ofstream logFile(logPath, std::ios::app);

        if (logFile.is_open()) {
            std::string timestamp = getTimestamp();
            logFile << "[" << timestamp << "] [" << level << "] " << message << std::endl;
            logFile.close();
        } else {
            std::cerr << "Failed to open log file: " << logPath << std::endl;
        }
    }

    void debug(const std::string& message) { log(message, "DEBUG"); }
    void info(const std::string& message) { log(message, "INFO"); }
    void warning(const std::string& message) { log(message, "WARNING"); }
    void error(const std::string& message) { log(message, "ERROR"); }
};

int main() {
    FileLogger logger("application");

    logger.info("Application started successfully.");
    logger.debug("Initializing configuration...");
    logger.warning("Resource usage is above 80%.");
    logger.error("Failed to connect to database.");

    return 0;
}