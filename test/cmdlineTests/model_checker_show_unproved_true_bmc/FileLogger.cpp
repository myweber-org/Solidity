
#include <fstream>
#include <filesystem>
#include <chrono>
#include <zlib.h>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

class FileLogger {
private:
    fs::path logDirectory;
    std::string baseName;
    size_t maxFileSize;
    int maxBackupCount;
    std::ofstream currentStream;
    size_t currentSize;

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
        return ss.str();
    }

    void rotateIfNeeded() {
        if (currentSize >= maxFileSize) {
            currentStream.close();
            std::string timestamp = getTimestamp();
            fs::path oldPath = logDirectory / (baseName + ".log");
            fs::path newPath = logDirectory / (baseName + "_" + timestamp + ".log");

            if (fs::exists(oldPath)) {
                fs::rename(oldPath, newPath);
                compressFile(newPath);
            }

            cleanupOldBackups();
            openNewFile();
        }
    }

    void compressFile(const fs::path& source) {
        fs::path dest = source.string() + ".gz";
        std::ifstream in(source, std::ios::binary);
        std::ofstream out(dest, std::ios::binary);

        const size_t bufferSize = 1024 * 1024;
        char* buffer = new char[bufferSize];
        z_stream stream = {};
        deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY);

        while (!in.eof()) {
            in.read(buffer, bufferSize);
            stream.avail_in = static_cast<uInt>(in.gcount());
            stream.next_in = reinterpret_cast<Bytef*>(buffer);

            do {
                char outBuffer[1024];
                stream.avail_out = sizeof(outBuffer);
                stream.next_out = reinterpret_cast<Bytef*>(outBuffer);
                deflate(&stream, Z_FINISH);
                size_t have = sizeof(outBuffer) - stream.avail_out;
                out.write(outBuffer, have);
            } while (stream.avail_out == 0);
        }

        deflateEnd(&stream);
        delete[] buffer;
        in.close();
        out.close();
        fs::remove(source);
    }

    void cleanupOldBackups() {
        std::vector<fs::path> backups;
        for (const auto& entry : fs::directory_iterator(logDirectory)) {
            if (entry.path().extension() == ".gz" && 
                entry.path().string().find(baseName) != std::string::npos) {
                backups.push_back(entry.path());
            }
        }

        std::sort(backups.begin(), backups.end());
        while (backups.size() > static_cast<size_t>(maxBackupCount)) {
            fs::remove(backups.front());
            backups.erase(backups.begin());
        }
    }

    void openNewFile() {
        fs::path filePath = logDirectory / (baseName + ".log");
        currentStream.open(filePath, std::ios::app);
        currentSize = fs::file_size(filePath);
    }

public:
    FileLogger(const std::string& dir, const std::string& name, size_t maxSize = 10485760, int backups = 10)
        : logDirectory(dir), baseName(name), maxFileSize(maxSize), maxBackupCount(backups), currentSize(0) {
        
        if (!fs::exists(logDirectory)) {
            fs::create_directories(logDirectory);
        }
        openNewFile();
    }

    ~FileLogger() {
        if (currentStream.is_open()) {
            currentStream.close();
        }
    }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        std::string entry = "[" + getTimestamp() + "] " + message + "\n";
        currentStream << entry;
        currentStream.flush();
        currentSize += entry.size();
        rotateIfNeeded();
    }

    void logWithLevel(const std::string& level, const std::string& message) {
        log("[" + level + "] " + message);
    }

private:
    std::mutex logMutex;
};