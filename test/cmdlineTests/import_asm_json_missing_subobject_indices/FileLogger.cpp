
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <filesystem>

class FileLogger {
private:
    std::ofstream logFile;
    std::mutex logMutex;
    std::string currentDate;
    std::string logDirectory;

    std::string getCurrentDate() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d");
        return ss.str();
    }

    void rotateLogIfNeeded() {
        std::string today = getCurrentDate();
        if (today != currentDate) {
            std::lock_guard<std::mutex> lock(logMutex);
            if (logFile.is_open()) {
                logFile.close();
            }
            currentDate = today;
            std::filesystem::create_directories(logDirectory);
            std::string filename = logDirectory + "/app_" + currentDate + ".log";
            logFile.open(filename, std::ios::app);
        }
    }

public:
    FileLogger(const std::string& directory = "./logs") : logDirectory(directory) {
        currentDate = getCurrentDate();
        std::filesystem::create_directories(logDirectory);
        std::string filename = logDirectory + "/app_" + currentDate + ".log";
        logFile.open(filename, std::ios::app);
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(const std::string& message, const std::string& level = "INFO") {
        rotateLogIfNeeded();
        
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::lock_guard<std::mutex> lock(logMutex);
        if (logFile.is_open()) {
            logFile << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
            logFile << "." << std::setfill('0') << std::setw(3) << ms.count();
            logFile << " [" << level << "] " << message << std::endl;
            logFile.flush();
        }
    }

    void info(const std::string& message) {
        log(message, "INFO");
    }

    void warning(const std::string& message) {
        log(message, "WARN");
    }

    void error(const std::string& message) {
        log(message, "ERROR");
    }
};