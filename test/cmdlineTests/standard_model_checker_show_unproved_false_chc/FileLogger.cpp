
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
        std::tm tm = *std::localtime(&time);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d");
        return oss.str();
    }

    void rotateLogIfNeeded() {
        std::string today = getCurrentDate();
        if (today != currentDate) {
            logFile.close();
            currentDate = today;
            openLogFile();
        }
    }

    void openLogFile() {
        std::filesystem::create_directories(logDirectory);
        std::string filename = logDirectory + "/app_" + currentDate + ".log";
        logFile.open(filename, std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Cannot open log file: " + filename);
        }
    }

public:
    FileLogger(const std::string& directory = "./logs") : logDirectory(directory) {
        currentDate = getCurrentDate();
        openLogFile();
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(const std::string& message, const std::string& level = "INFO") {
        std::lock_guard<std::mutex> lock(logMutex);
        rotateLogIfNeeded();

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&time);
        
        logFile << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        logFile << " [" << level << "] " << message << std::endl;
        logFile.flush();
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