
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>

class FileLogger {
private:
    std::string logDirectory;
    std::string baseFilename;
    size_t maxFileSize;
    int maxFiles;
    std::ofstream currentStream;
    int currentFileIndex;
    size_t currentSize;

    std::string generateTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
        return ss.str();
    }

    std::string constructFilename(int index) {
        std::stringstream ss;
        ss << logDirectory << "/" << baseFilename << "_" << generateTimestamp();
        if (index > 0) {
            ss << "_" << index;
        }
        ss << ".log";
        return ss.str();
    }

    void rotateIfNeeded() {
        if (currentSize >= maxFileSize) {
            currentStream.close();
            currentFileIndex++;
            
            if (currentFileIndex >= maxFiles) {
                currentFileIndex = 0;
            }
            
            std::string newFilename = constructFilename(currentFileIndex);
            currentStream.open(newFilename, std::ios::out | std::ios::trunc);
            currentSize = 0;
        }
    }

public:
    FileLogger(const std::string& directory = "logs", 
               const std::string& filename = "app",
               size_t maxSize = 1048576, 
               int maxCount = 5)
        : logDirectory(directory), baseFilename(filename), 
          maxFileSize(maxSize), maxFiles(maxCount),
          currentFileIndex(0), currentSize(0) {
        
        std::filesystem::create_directories(logDirectory);
        std::string initialFile = constructFilename(currentFileIndex);
        currentStream.open(initialFile, std::ios::out | std::ios::app);
    }

    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }

    void log(const std::string& message, const std::string& level = "INFO") {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        currentStream << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        currentStream << " [" << level << "] " << message << std::endl;
        
        currentSize = currentStream.tellp();
        rotateIfNeeded();
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