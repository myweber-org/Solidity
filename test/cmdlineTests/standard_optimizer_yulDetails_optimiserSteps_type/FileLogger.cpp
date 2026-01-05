
#include <fstream>
#include <filesystem>
#include <chrono>
#include <sstream>
#include <zlib.h>
#include <iomanip>

namespace fs = std::filesystem;

class FileLogger {
private:
    fs::path basePath;
    size_t maxFileSize;
    int maxBackupCount;
    std::ofstream currentStream;
    std::string currentFilename;
    
    std::string generateTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
        return ss.str();
    }
    
    void rotateIfNeeded() {
        if (!currentStream.is_open()) return;
        
        currentStream.seekp(0, std::ios::end);
        size_t currentSize = currentStream.tellp();
        
        if (currentSize >= maxFileSize) {
            currentStream.close();
            compressCurrentFile();
            cleanupOldBackups();
            openNewFile();
        }
    }
    
    void compressCurrentFile() {
        if (!fs::exists(currentFilename)) return;
        
        std::ifstream input(currentFilename, std::ios::binary);
        if (!input) return;
        
        std::string compressedName = currentFilename + ".gz";
        gzFile output = gzopen(compressedName.c_str(), "wb");
        if (!output) return;
        
        char buffer[8192];
        while (input.read(buffer, sizeof(buffer)) || input.gcount()) {
            gzwrite(output, buffer, input.gcount());
        }
        
        gzclose(output);
        input.close();
        fs::remove(currentFilename);
    }
    
    void cleanupOldBackups() {
        std::vector<fs::path> backups;
        for (const auto& entry : fs::directory_iterator(basePath)) {
            if (entry.path().extension() == ".gz") {
                backups.push_back(entry.path());
            }
        }
        
        std::sort(backups.begin(), backups.end(), 
                 [](const fs::path& a, const fs::path& b) {
                     return fs::last_write_time(a) < fs::last_write_time(b);
                 });
        
        while (backups.size() > static_cast<size_t>(maxBackupCount)) {
            fs::remove(backups.front());
            backups.erase(backups.begin());
        }
    }
    
    void openNewFile() {
        currentFilename = (basePath / ("log_" + generateTimestamp() + ".txt")).string();
        currentStream.open(currentFilename, std::ios::app);
    }
    
public:
    FileLogger(const std::string& path, size_t maxSize = 1048576, int backups = 10)
        : basePath(path), maxFileSize(maxSize), maxBackupCount(backups) {
        
        if (!fs::exists(basePath)) {
            fs::create_directories(basePath);
        }
        
        openNewFile();
    }
    
    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }
    
    void log(const std::string& message) {
        rotateIfNeeded();
        
        if (currentStream.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            currentStream << std::put_time(std::localtime(&time), "[%Y-%m-%d %H:%M:%S] ")
                         << message << std::endl;
        }
    }
    
    void flush() {
        if (currentStream.is_open()) {
            currentStream.flush();
        }
    }
};