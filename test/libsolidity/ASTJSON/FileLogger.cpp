#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <zlib.h>
#include <vector>

namespace fs = std::filesystem;

class FileLogger {
private:
    fs::path logDirectory;
    std::string baseFilename;
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

    void compressFile(const fs::path& source, const fs::path& dest) {
        std::ifstream inFile(source, std::ios::binary);
        if (!inFile) return;

        std::vector<char> buffer(std::istreambuf_iterator<char>(inFile), {});
        inFile.close();

        uLongf compressedSize = compressBound(buffer.size());
        std::vector<Bytef> compressed(compressedSize);

        if (compress(compressed.data(), &compressedSize,
                     reinterpret_cast<Bytef*>(buffer.data()), buffer.size()) == Z_OK) {
            std::ofstream outFile(dest, std::ios::binary);
            outFile.write(reinterpret_cast<char*>(compressed.data()), compressedSize);
            outFile.close();
            fs::remove(source);
        }
    }

    void rotateLog() {
        if (currentStream.is_open()) {
            currentStream.close();
        }

        for (int i = maxBackupFiles - 1; i > 0; --i) {
            fs::path oldFile = logDirectory / (baseFilename + "." + std::to_string(i) + ".gz");
            fs::path newFile = logDirectory / (baseFilename + "." + std::to_string(i + 1) + ".gz");
            if (fs::exists(oldFile)) {
                fs::rename(oldFile, newFile);
            }
        }

        fs::path firstBackup = logDirectory / (baseFilename + ".1.gz");
        fs::path currentLog = logDirectory / (baseFilename + ".log");
        
        if (fs::exists(currentLog)) {
            compressFile(currentLog, firstBackup);
        }

        openCurrentLog();
    }

    void openCurrentLog() {
        fs::path currentPath = logDirectory / (baseFilename + ".log");
        currentStream.open(currentPath, std::ios::app);
        currentSize = fs::file_size(currentPath);
    }

public:
    FileLogger(const std::string& dir, const std::string& filename,
               size_t maxSize = 10485760, int backups = 5)
        : logDirectory(dir), baseFilename(filename),
          maxFileSize(maxSize), maxBackupFiles(backups), currentSize(0) {
        
        if (!fs::exists(logDirectory)) {
            fs::create_directories(logDirectory);
        }

        openCurrentLog();
    }

    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }

    void log(const std::string& message, const std::string& level = "INFO") {
        std::string entry = "[" + getTimestamp() + "] [" + level + "] " + message + "\n";
        
        if (currentSize + entry.size() > maxFileSize) {
            rotateLog();
        }

        currentStream << entry;
        currentStream.flush();
        currentSize += entry.size();
    }

    void debug(const std::string& msg) { log(msg, "DEBUG"); }
    void info(const std::string& msg) { log(msg, "INFO"); }
    void warning(const std::string& msg) { log(msg, "WARNING"); }
    void error(const std::string& msg) { log(msg, "ERROR"); }
};

void exampleUsage() {
    FileLogger logger("logs", "application");
    
    logger.info("Application started");
    logger.debug("Initializing components");
    
    for (int i = 0; i < 1000; ++i) {
        logger.info("Processing item " + std::to_string(i));
    }
    
    logger.error("Simulated error occurred");
    logger.info("Application shutting down");
}

int main() {
    exampleUsage();
    return 0;
}
#include <fstream>
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

class FileLogger {
public:
    FileLogger(const std::string& filename) : logFile(filename, std::ios::app) {
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file: " << filename << std::endl;
        }
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(LogLevel level, const std::string& message) {
        if (!logFile.is_open()) return;

        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm local_tm = *std::localtime(&now_time);

        logFile << "[" << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << "] ";

        switch (level) {
            case LogLevel::INFO:
                logFile << "[INFO] ";
                break;
            case LogLevel::WARNING:
                logFile << "[WARNING] ";
                break;
            case LogLevel::ERROR:
                logFile << "[ERROR] ";
                break;
        }

        logFile << message << std::endl;
    }

    void info(const std::string& message) {
        log(LogLevel::INFO, message);
    }

    void warning(const std::string& message) {
        log(LogLevel::WARNING, message);
    }

    void error(const std::string& message) {
        log(LogLevel::ERROR, message);
    }

private:
    std::ofstream logFile;
};

int main() {
    FileLogger logger("application.log");

    logger.info("Application started successfully.");
    logger.warning("Disk space is running low.");
    logger.error("Failed to connect to database.");

    return 0;
}