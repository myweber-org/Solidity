
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class Logger {
private:
    std::ofstream logFile;
    std::string baseFilename;
    size_t maxFileSize;
    int maxBackupFiles;
    std::string currentLogPath;

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

    void rotateIfNeeded() {
        if (!logFile.is_open()) return;

        logFile.seekp(0, std::ios::end);
        size_t currentSize = logFile.tellp();

        if (currentSize >= maxFileSize) {
            logFile.close();
            performRotation();
            openLogFile();
        }
    }

    void performRotation() {
        std::vector<std::string> backupFiles;
        
        for (const auto& entry : fs::directory_iterator(fs::path(currentLogPath).parent_path())) {
            std::string filename = entry.path().filename().string();
            if (filename.find(baseFilename) == 0 && filename != baseFilename) {
                backupFiles.push_back(filename);
            }
        }

        std::sort(backupFiles.begin(), backupFiles.end());

        for (int i = backupFiles.size() - 1; i >= 0; --i) {
            std::string oldPath = fs::path(currentLogPath).parent_path() / backupFiles[i];
            std::string newPath = fs::path(currentLogPath).parent_path() / 
                                 (baseFilename + "." + std::to_string(i + 2));
            
            if (i + 1 >= maxBackupFiles) {
                fs::remove(oldPath);
            } else {
                fs::rename(oldPath, newPath);
            }
        }

        std::string firstBackup = fs::path(currentLogPath).parent_path() / 
                                 (baseFilename + ".1");
        fs::rename(currentLogPath, firstBackup);
    }

    void openLogFile() {
        logFile.open(currentLogPath, std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Cannot open log file: " + currentLogPath);
        }
    }

public:
    Logger(const std::string& filename = "application.log", 
           size_t maxSize = 1048576, 
           int maxBackups = 5)
        : baseFilename(filename), maxFileSize(maxSize), maxBackupFiles(maxBackups) {
        
        currentLogPath = fs::current_path() / baseFilename;
        openLogFile();
    }

    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(const std::string& level, const std::string& message) {
        rotateIfNeeded();
        
        std::string timestamp = getTimestamp();
        std::string logEntry = "[" + timestamp + "] [" + level + "] " + message + "\n";
        
        logFile << logEntry;
        logFile.flush();
        
        std::cout << logEntry;
    }

    void info(const std::string& message) {
        log("INFO", message);
    }

    void warning(const std::string& message) {
        log("WARNING", message);
    }

    void error(const std::string& message) {
        log("ERROR", message);
    }

    void debug(const std::string& message) {
        log("DEBUG", message);
    }
};

void exampleUsage() {
    Logger logger("myapp.log", 1024, 3);
    
    logger.info("Application started");
    logger.debug("Initializing components");
    logger.warning("Configuration file not found, using defaults");
    logger.error("Failed to connect to database");
    logger.info("Application shutdown");
}

int main() {
    try {
        exampleUsage();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Logger error: " << e.what() << std::endl;
        return 1;
    }
}