
#include <fstream>
#include <string>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

class FileLogger {
private:
    std::ofstream logFile;
    std::string baseFilename;
    std::string currentFilename;
    size_t maxFileSize;
    int maxBackupFiles;
    size_t currentSize;

    std::string generateTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
        return ss.str();
    }

    void rotateIfNeeded() {
        if (currentSize >= maxFileSize) {
            logFile.close();
            
            for (int i = maxBackupFiles - 1; i > 0; --i) {
                std::string oldName = baseFilename + "." + std::to_string(i);
                std::string newName = baseFilename + "." + std::to_string(i + 1);
                if (fs::exists(oldName)) {
                    fs::rename(oldName, newName);
                }
            }
            
            std::string firstBackup = baseFilename + ".1";
            if (fs::exists(currentFilename)) {
                fs::rename(currentFilename, firstBackup);
            }
            
            openNewLogFile();
        }
    }

    void openNewLogFile() {
        currentFilename = baseFilename;
        logFile.open(currentFilename, std::ios::app);
        currentSize = 0;
        
        if (logFile.is_open()) {
            std::string timestamp = generateTimestamp();
            logFile << "=== Log session started at " << timestamp << " ===" << std::endl;
            currentSize = logFile.tellp();
        }
    }

public:
    FileLogger(const std::string& filename, size_t maxSize = 1048576, int maxBackups = 5)
        : baseFilename(filename), maxFileSize(maxSize), maxBackupFiles(maxBackups), currentSize(0) {
        openNewLogFile();
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(const std::string& message, const std::string& level = "INFO") {
        if (!logFile.is_open()) return;
        
        rotateIfNeeded();
        
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        logFile << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        logFile << " [" << level << "] " << message << std::endl;
        
        currentSize = logFile.tellp();
    }

    void info(const std::string& message) {
        log(message, "INFO");
    }

    void warning(const std::string& message) {
        log(message, "WARN");
    }

    void error(const std::string& message) {
        log(message, "ERROR");
    }

    void flush() {
        if (logFile.is_open()) {
            logFile.flush();
        }
    }
};