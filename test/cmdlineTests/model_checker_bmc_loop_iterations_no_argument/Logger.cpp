
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

class Logger {
public:
    enum class Level {
        INFO,
        WARNING,
        ERROR
    };

    Logger(const std::string& filename) {
        logFile.open(filename, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file: " << filename << std::endl;
        }
    }

    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(Level level, const std::string& message) {
        std::string timestamp = getCurrentTimestamp();
        std::string levelStr = levelToString(level);
        
        std::string logEntry = "[" + timestamp + "] [" + levelStr + "] " + message;
        
        if (logFile.is_open()) {
            logFile << logEntry << std::endl;
        }
        
        std::cout << logEntry << std::endl;
    }

    void info(const std::string& message) {
        log(Level::INFO, message);
    }

    void warning(const std::string& message) {
        log(Level::WARNING, message);
    }

    void error(const std::string& message) {
        log(Level::ERROR, message);
    }

private:
    std::ofstream logFile;

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

    std::string levelToString(Level level) {
        switch (level) {
            case Level::INFO: return "INFO";
            case Level::WARNING: return "WARNING";
            case Level::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }
};

void exampleUsage() {
    Logger logger("application.log");
    
    logger.info("Application started");
    logger.warning("Disk space is running low");
    logger.error("Failed to connect to database");
    
    for (int i = 0; i < 3; i++) {
        logger.info("Processing iteration " + std::to_string(i));
    }
    
    logger.info("Application shutdown");
}

int main() {
    exampleUsage();
    return 0;
}