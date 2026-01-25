
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

class FileLogger {
private:
    std::ofstream logFile;
    std::mutex logMutex;
    std::string filename;

    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

public:
    explicit FileLogger(const std::string& filepath) : filename(filepath) {
        logFile.open(filename, std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Failed to open log file: " + filename);
        }
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(const std::string& message, const std::string& level = "INFO") {
        std::lock_guard<std::mutex> lock(logMutex);
        if (logFile.is_open()) {
            logFile << "[" << getCurrentTimestamp() << "] "
                    << "[" << level << "] "
                    << message << std::endl;
            logFile.flush();
        }
    }

    void error(const std::string& message) {
        log(message, "ERROR");
    }

    void warning(const std::string& message) {
        log(message, "WARNING");
    }

    void info(const std::string& message) {
        log(message, "INFO");
    }

    void debug(const std::string& message) {
        log(message, "DEBUG");
    }
};