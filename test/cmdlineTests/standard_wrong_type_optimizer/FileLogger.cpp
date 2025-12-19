#include <fstream>
#include <string>
#include <ctime>
#include <filesystem>
#include <sstream>
#include <iomanip>

class FileLogger {
private:
    std::string logDirectory;
    std::string baseFilename;
    size_t maxFileSize;
    int maxBackupFiles;
    std::ofstream currentStream;
    std::string currentFilePath;
    size_t currentFileSize;

    std::string generateTimestamp() {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
        return oss.str();
    }

    void rotateIfNeeded() {
        if (currentFileSize >= maxFileSize && currentStream.is_open()) {
            currentStream.close();
            std::string newPath = currentFilePath + "." + generateTimestamp();
            std::filesystem::rename(currentFilePath, newPath);
            openNewLogFile();
            cleanupOldFiles();
        }
    }

    void openNewLogFile() {
        currentFilePath = logDirectory + "/" + baseFilename + ".log";
        currentStream.open(currentFilePath, std::ios::app);
        currentFileSize = std::filesystem::exists(currentFilePath) ? std::filesystem::file_size(currentFilePath) : 0;
    }

    void cleanupOldFiles() {
        std::vector<std::filesystem::path> backupFiles;
        for (const auto& entry : std::filesystem::directory_iterator(logDirectory)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename.find(baseFilename + ".log.") == 0) {
                    backupFiles.push_back(entry.path());
                }
            }
        }
        std::sort(backupFiles.begin(), backupFiles.end(),
                  [](const std::filesystem::path& a, const std::filesystem::path& b) {
                      return std::filesystem::last_write_time(a) < std::filesystem::last_write_time(b);
                  });
        while (backupFiles.size() > static_cast<size_t>(maxBackupFiles)) {
            std::filesystem::remove(backupFiles.front());
            backupFiles.erase(backupFiles.begin());
        }
    }

public:
    FileLogger(const std::string& dir = "logs",
               const std::string& filename = "app",
               size_t maxSize = 1048576,
               int maxBackups = 5)
        : logDirectory(dir), baseFilename(filename), maxFileSize(maxSize), maxBackupFiles(maxBackups), currentFileSize(0) {
        std::filesystem::create_directories(logDirectory);
        openNewLogFile();
    }

    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }

    void log(const std::string& level, const std::string& message) {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream timestamp;
        timestamp << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

        std::ostringstream logEntry;
        logEntry << "[" << timestamp.str() << "] [" << level << "] " << message << "\n";
        std::string entryStr = logEntry.str();

        rotateIfNeeded();
        if (currentStream.is_open()) {
            currentStream << entryStr;
            currentStream.flush();
            currentFileSize += entryStr.size();
        }
    }

    void info(const std::string& message) {
        log("INFO", message);
    }

    void warn(const std::string& message) {
        log("WARN", message);
    }

    void error(const std::string& message) {
        log("ERROR", message);
    }
};