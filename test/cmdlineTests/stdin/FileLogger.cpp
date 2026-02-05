
#include <fstream>
#include <string>
#include <chrono>
#include <iomanop>
#include <filesystem>
#include <mutex>

namespace utility {

    enum class LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    class FileLogger {
    private:
        std::ofstream logFile;
        std::string filePath;
        std::string baseName;
        std::string extension;
        size_t maxFileSize;
        int maxBackupFiles;
        LogLevel currentLevel;
        std::mutex writeMutex;

        std::string getTimestamp() {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

            std::ostringstream oss;
            oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
            oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
            return oss.str();
        }

        std::string levelToString(LogLevel level) {
            switch(level) {
                case LogLevel::DEBUG: return "DEBUG";
                case LogLevel::INFO: return "INFO";
                case LogLevel::WARNING: return "WARNING";
                case LogLevel::ERROR: return "ERROR";
                default: return "UNKNOWN";
            }
        }

        void rotateIfNeeded() {
            if (!logFile.is_open()) return;

            logFile.seekp(0, std::ios::end);
            size_t currentSize = logFile.tellp();

            if (currentSize >= maxFileSize) {
                logFile.close();

                for (int i = maxBackupFiles - 1; i > 0; --i) {
                    std::string oldName = baseName + "." + std::to_string(i) + extension;
                    std::string newName = baseName + "." + std::to_string(i + 1) + extension;

                    if (std::filesystem::exists(oldName)) {
                        std::filesystem::rename(oldName, newName);
                    }
                }

                std::string firstBackup = baseName + ".1" + extension;
                std::filesystem::rename(filePath, firstBackup);

                logFile.open(filePath, std::ios::out | std::ios::app);
            }
        }

    public:
        FileLogger(const std::string& path, 
                   LogLevel level = LogLevel::INFO,
                   size_t maxSize = 10485760, // 10MB
                   int backups = 5)
            : filePath(path), maxFileSize(maxSize), maxBackupFiles(backups), currentLevel(level) {
            
            std::filesystem::path fsPath(path);
            baseName = fsPath.parent_path() / fsPath.stem();
            extension = fsPath.extension();

            logFile.open(filePath, std::ios::out | std::ios::app);
            if (!logFile.is_open()) {
                throw std::runtime_error("Cannot open log file: " + path);
            }
        }

        ~FileLogger() {
            if (logFile.is_open()) {
                logFile.close();
            }
        }

        void setLogLevel(LogLevel level) {
            std::lock_guard<std::mutex> lock(writeMutex);
            currentLevel = level;
        }

        void log(LogLevel level, const std::string& message) {
            if (level < currentLevel) return;

            std::lock_guard<std::mutex> lock(writeMutex);
            
            rotateIfNeeded();
            
            logFile << "[" << getTimestamp() << "] "
                    << "[" << levelToString(level) << "] "
                    << message << std::endl;
            
            logFile.flush();
        }

        void debug(const std::string& message) {
            log(LogLevel::DEBUG, message);
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
    };

} // namespace utility