
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
    std::string logFileBaseName;
    size_t maxFileSize;
    int maxBackupFiles;
    std::ofstream currentLogStream;
    size_t currentFileSize;

    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
        return ss.str();
    }

    void rotateLogIfNeeded() {
        if (currentFileSize >= maxFileSize) {
            currentLogStream.close();
            std::string timestamp = getCurrentTimestamp();
            fs::path oldPath = logDirectory / (logFileBaseName + ".log");
            fs::path newPath = logDirectory / (logFileBaseName + "_" + timestamp + ".log");
            
            if (fs::exists(oldPath)) {
                fs::rename(oldPath, newPath);
                compressFile(newPath);
            }
            
            manageBackupFiles();
            openNewLogFile();
        }
    }

    void compressFile(const fs::path& filePath) {
        std::ifstream input(filePath, std::ios::binary);
        if (!input) return;

        std::vector<char> buffer(std::istreambuf_iterator<char>(input), {});
        input.close();

        uLongf compressedSize = compressBound(buffer.size());
        std::vector<Bytef> compressedBuffer(compressedSize);

        if (compress(compressedBuffer.data(), &compressedSize, 
                    reinterpret_cast<Bytef*>(buffer.data()), buffer.size()) == Z_OK) {
            fs::path compressedPath = filePath;
            compressedPath += ".gz";
            
            std::ofstream output(compressedPath, std::ios::binary);
            output.write(reinterpret_cast<char*>(compressedBuffer.data()), compressedSize);
            output.close();
            
            fs::remove(filePath);
        }
    }

    void manageBackupFiles() {
        std::vector<fs::path> backupFiles;
        for (const auto& entry : fs::directory_iterator(logDirectory)) {
            if (entry.path().extension() == ".gz" && 
                entry.path().string().find(logFileBaseName) != std::string::npos) {
                backupFiles.push_back(entry.path());
            }
        }

        std::sort(backupFiles.begin(), backupFiles.end());
        while (backupFiles.size() > static_cast<size_t>(maxBackupFiles)) {
            fs::remove(backupFiles.front());
            backupFiles.erase(backupFiles.begin());
        }
    }

    void openNewLogFile() {
        fs::path logFilePath = logDirectory / (logFileBaseName + ".log");
        currentLogStream.open(logFilePath, std::ios::app);
        currentFileSize = fs::exists(logFilePath) ? fs::file_size(logFilePath) : 0;
    }

public:
    FileLogger(const std::string& directory, const std::string& baseName, 
               size_t maxSize = 1048576, int maxBackups = 10)
        : logDirectory(directory), logFileBaseName(baseName), 
          maxFileSize(maxSize), maxBackupFiles(maxBackups), currentFileSize(0) {
        
        if (!fs::exists(logDirectory)) {
            fs::create_directories(logDirectory);
        }
        
        openNewLogFile();
    }

    ~FileLogger() {
        if (currentLogStream.is_open()) {
            currentLogStream.close();
        }
    }

    void log(const std::string& message, const std::string& level = "INFO") {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        
        std::string logEntry = "[" + ss.str() + "] [" + level + "] " + message + "\n";
        
        std::lock_guard<std::mutex> lock(logMutex);
        currentLogStream << logEntry;
        currentLogStream.flush();
        currentFileSize += logEntry.size();
        
        rotateLogIfNeeded();
    }

    void logError(const std::string& message) {
        log(message, "ERROR");
    }

    void logWarning(const std::string& message) {
        log(message, "WARNING");
    }

    void logDebug(const std::string& message) {
        log(message, "DEBUG");
    }

private:
    std::mutex logMutex;
};