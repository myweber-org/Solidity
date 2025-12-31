
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class FileLogger {
public:
    explicit FileLogger(const std::string& filename) : logFile(filename, std::ios::app) {}
    
    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }
    
    void log(LogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        if (!logFile.is_open()) return;
        
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        logFile << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        logFile << '.' << std::setfill('0') << std::setw(3) << ms.count() << " ";
        
        switch (level) {
            case LogLevel::DEBUG: logFile << "[DEBUG] "; break;
            case LogLevel::INFO: logFile << "[INFO] "; break;
            case LogLevel::WARNING: logFile << "[WARNING] "; break;
            case LogLevel::ERROR: logFile << "[ERROR] "; break;
        }
        
        logFile << message << std::endl;
    }
    
    void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    void info(const std::string& message) { log(LogLevel::INFO, message); }
    void warning(const std::string& message) { log(LogLevel::WARNING, message); }
    void error(const std::string& message) { log(LogLevel::ERROR, message); }
    
private:
    std::ofstream logFile;
    std::mutex logMutex;
};