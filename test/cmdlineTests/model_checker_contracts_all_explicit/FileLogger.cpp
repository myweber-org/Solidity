
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
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
    size_t currentFileSize;

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
        return ss.str();
    }

    void compressFile(const fs::path& source) {
        std::ifstream input(source, std::ios::binary);
        if (!input) return;

        std::vector<char> buffer(std::istreambuf_iterator<char>(input), {});
        input.close();

        uLongf compressedSize = compressBound(buffer.size());
        std::vector<Bytef> compressed(compressedSize);

        if (compress(compressed.data(), &compressedSize,
                    reinterpret_cast<Bytef*>(buffer.data()), buffer.size()) == Z_OK) {
            fs::path dest = source;
            dest += ".gz";
            std::ofstream output(dest, std::ios::binary);
            output.write(reinterpret_cast<char*>(compressed.data()), compressedSize);
            output.close();
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

        fs::path firstBackup = logDirectory / (baseFilename + ".1");
        if (fs::exists(firstBackup)) {
            compressFile(firstBackup);
        }

        fs::path currentLog = logDirectory / baseFilename;
        if (fs::exists(currentLog)) {
            fs::rename(currentLog, firstBackup);
        }

        openCurrentLog();
    }

    void openCurrentLog() {
        fs::path currentPath = logDirectory / baseFilename;
        currentStream.open(currentPath, std::ios::app);
        currentFileSize = fs::file_size(currentPath);
    }

public:
    FileLogger(const std::string& directory, const std::string& filename,
               size_t maxSize = 10485760, int maxBackups = 5)
        : logDirectory(directory), baseFilename(filename),
          maxFileSize(maxSize), maxBackupFiles(maxBackups), currentFileSize(0) {
        
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

    void log(const std::string& message) {
        std::string entry = "[" + getTimestamp() + "] " + message + "\n";
        
        if (currentFileSize + entry.size() > maxFileSize) {
            rotateLog();
        }
        
        currentStream << entry;
        currentStream.flush();
        currentFileSize += entry.size();
    }

    void logWithLevel(const std::string& level, const std::string& message) {
        log("[" + level + "] " + message);
    }
};