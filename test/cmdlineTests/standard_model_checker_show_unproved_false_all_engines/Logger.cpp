
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <mutex>

namespace Logger {
    enum class LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };

    class Logger {
    private:
        std::ofstream logFile;
        LogLevel currentLevel;
        std::string filename;
        size_t maxFileSize;
        int maxBackupFiles;
        std::mutex logMutex;

        std::string getTimestamp() {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) % 1000;

            std::stringstream ss;
            ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
            ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
            return ss.str();
        }

        std::string levelToString(LogLevel level) {
            switch(level) {
                case LogLevel::DEBUG: return "DEBUG";
                case LogLevel::INFO: return "INFO";
                case LogLevel::WARNING: return "WARNING";
                case LogLevel::ERROR: return "ERROR";
                case LogLevel::CRITICAL: return "CRITICAL";
                default: return "UNKNOWN";
            }
        }

        void rotateLogFile() {
            logFile.close();
            
            for (int i = maxBackupFiles - 1; i > 0; --i) {
                std::string oldName = filename + "." + std::to_string(i);
                std::string newName = filename + "." + std::to_string(i + 1);
                
                if (std::filesystem::exists(oldName)) {
                    std::filesystem::rename(oldName, newName);
                }
            }
            
            std::string firstBackup = filename + ".1";
            std::filesystem::rename(filename, firstBackup);
            
            logFile.open(filename, std::ios::out | std::ios::app);
        }

        bool shouldRotate() {
            if (!logFile.is_open()) return false;
            
            logFile.seekp(0, std::ios::end);
            size_t size = logFile.tellp();
            return size >= maxFileSize;
        }

    public:
        Logger(const std::string& file = "application.log", 
               LogLevel level = LogLevel::INFO,
               size_t maxSize = 10485760, // 10MB
               int maxBackups = 5)
            : filename(file), currentLevel(level), 
              maxFileSize(maxSize), maxBackupFiles(maxBackups) {
            
            logFile.open(filename, std::ios::out | std::ios::app);
            if (!logFile.is_open()) {
                std::cerr << "Failed to open log file: " << filename << std::endl;
            }
        }

        ~Logger() {
            if (logFile.is_open()) {
                logFile.close();
            }
        }

        void setLogLevel(LogLevel level) {
            std::lock_guard<std::mutex> lock(logMutex);
            currentLevel = level;
        }

        void log(LogLevel level, const std::string& message) {
            if (level < currentLevel) return;

            std::lock_guard<std::mutex> lock(logMutex);
            
            if (shouldRotate()) {
                rotateLogFile();
            }

            std::string timestamp = getTimestamp();
            std::string levelStr = levelToString(level);
            
            std::string logEntry = "[" + timestamp + "] [" + levelStr + "] " + message;
            
            if (logFile.is_open()) {
                logFile << logEntry << std::endl;
                logFile.flush();
            }
            
            std::cout << logEntry << std::endl;
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

        void critical(const std::string& message) {
            log(LogLevel::CRITICAL, message);
        }
    };
}

int main() {
    Logger::Logger logger("app.log", Logger::LogLevel::DEBUG);
    
    logger.debug("Application started");
    logger.info("Initializing components");
    logger.warning("Configuration file not found, using defaults");
    logger.error("Failed to connect to database");
    logger.critical("System shutdown due to critical error");
    
    logger.setLogLevel(Logger::LogLevel::WARNING);
    logger.debug("This debug message will not be logged");
    logger.warning("This warning will be logged");
    
    return 0;
}