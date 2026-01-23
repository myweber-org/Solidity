
#include <fstream>
#include <string>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

class FileLogger {
private:
    std::string logDir;
    std::string baseName;
    size_t maxFileSize;
    int maxFiles;
    std::ofstream currentFile;
    size_t currentSize;

    std::string generateTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
        return ss.str();
    }

    void rotateIfNeeded() {
        if (currentSize >= maxFileSize && currentFile.is_open()) {
            currentFile.close();
            std::string newName = logDir + "/" + baseName + "_" + generateTimestamp() + ".log";
            fs::rename(logDir + "/" + baseName + ".log", newName);
            cleanupOldFiles();
            openNewFile();
        }
    }

    void cleanupOldFiles() {
        std::vector<fs::path> logFiles;
        for (const auto& entry : fs::directory_iterator(logDir)) {
            if (entry.path().extension() == ".log" && 
                entry.path().filename().string().find(baseName) != std::string::npos) {
                logFiles.push_back(entry.path());
            }
        }
        
        std::sort(logFiles.begin(), logFiles.end());
        while (logFiles.size() > static_cast<size_t>(maxFiles)) {
            fs::remove(logFiles.front());
            logFiles.erase(logFiles.begin());
        }
    }

    void openNewFile() {
        std::string filePath = logDir + "/" + baseName + ".log";
        currentFile.open(filePath, std::ios::app);
        currentSize = fs::exists(filePath) ? fs::file_size(filePath) : 0;
    }

public:
    FileLogger(const std::string& directory, const std::string& name, 
               size_t maxSize = 1048576, int maxCount = 10)
        : logDir(directory), baseName(name), maxFileSize(maxSize), maxFiles(maxCount), currentSize(0) {
        
        if (!fs::exists(logDir)) {
            fs::create_directories(logDir);
        }
        
        openNewFile();
    }

    ~FileLogger() {
        if (currentFile.is_open()) {
            currentFile.close();
        }
    }

    void log(const std::string& message) {
        rotateIfNeeded();
        
        if (currentFile.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&time), "[%Y-%m-%d %H:%M:%S] ");
            ss << message << std::endl;
            
            std::string logEntry = ss.str();
            currentFile << logEntry;
            currentFile.flush();
            currentSize += logEntry.size();
        }
    }

    void logError(const std::string& errorMessage) {
        log("ERROR: " + errorMessage);
    }

    void logWarning(const std::string& warningMessage) {
        log("WARNING: " + warningMessage);
    }

    void logInfo(const std::string& infoMessage) {
        log("INFO: " + infoMessage);
    }
};