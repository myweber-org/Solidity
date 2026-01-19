
#include <fstream>
#include <string>
#include <chrono>
#include <filesystem>
#include <mutex>

class FileLogger {
private:
    std::string logDirectory;
    std::string baseFilename;
    size_t maxFileSize;
    int maxFiles;
    std::mutex logMutex;
    std::ofstream currentStream;
    std::string currentPath;
    size_t currentSize;

    std::string generateTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        char buffer[20];
        std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", std::localtime(&time));
        return std::string(buffer);
    }

    void rotateIfNeeded() {
        if (currentSize >= maxFileSize && currentStream.is_open()) {
            currentStream.close();
            
            std::string newPath = logDirectory + "/" + baseFilename + "_" + generateTimestamp() + ".log";
            std::filesystem::rename(currentPath, newPath);
            
            cleanupOldFiles();
            openNewLogFile();
        }
    }

    void cleanupOldFiles() {
        try {
            std::vector<std::filesystem::path> logFiles;
            for (const auto& entry : std::filesystem::directory_iterator(logDirectory)) {
                if (entry.is_regular_file() && 
                    entry.path().filename().string().find(baseFilename) == 0) {
                    logFiles.push_back(entry.path());
                }
            }
            
            std::sort(logFiles.begin(), logFiles.end(),
                     [](const std::filesystem::path& a, const std::filesystem::path& b) {
                         return std::filesystem::last_write_time(a) > 
                                std::filesystem::last_write_time(b);
                     });
            
            for (size_t i = maxFiles; i < logFiles.size(); ++i) {
                std::filesystem::remove(logFiles[i]);
            }
        } catch (const std::exception& e) {
            // Silently continue if cleanup fails
        }
    }

    void openNewLogFile() {
        currentPath = logDirectory + "/" + baseFilename + ".log";
        currentStream.open(currentPath, std::ios::app);
        currentSize = std::filesystem::file_size(currentPath);
    }

public:
    FileLogger(const std::string& directory, const std::string& filename, 
               size_t maxSize = 1048576, int keepFiles = 10)
        : logDirectory(directory), baseFilename(filename), 
          maxFileSize(maxSize), maxFiles(keepFiles), currentSize(0) {
        
        std::filesystem::create_directories(logDirectory);
        openNewLogFile();
    }

    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        
        if (!currentStream.is_open()) {
            return;
        }
        
        std::string timestamp = generateTimestamp();
        std::string logEntry = "[" + timestamp + "] " + message + "\n";
        
        currentStream << logEntry;
        currentStream.flush();
        
        currentSize += logEntry.size();
        rotateIfNeeded();
    }

    void logWithLevel(const std::string& level, const std::string& message) {
        log(level + ": " + message);
    }
};