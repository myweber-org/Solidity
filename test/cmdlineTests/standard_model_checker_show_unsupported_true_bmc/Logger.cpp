
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <ctime>
#include <memory>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    Logger(const std::string& name, LogLevel minLevel = LogLevel::INFO)
        : name_(name), minLevel_(minLevel), outputStream_(&std::clog) {}

    void setMinLevel(LogLevel level) { minLevel_ = level; }
    void setOutputStream(std::ostream& stream) { outputStream_ = &stream; }
    void setLogFile(const std::string& filename) {
        fileStream_ = std::make_unique<std::ofstream>(filename, std::ios::app);
        if (fileStream_->is_open()) {
            outputStream_ = fileStream_.get();
        }
    }

    void log(LogLevel level, const std::string& message) {
        if (level < minLevel_) return;

        std::string levelStr;
        switch (level) {
            case LogLevel::DEBUG:   levelStr = "DEBUG"; break;
            case LogLevel::INFO:    levelStr = "INFO"; break;
            case LogLevel::WARNING: levelStr = "WARN"; break;
            case LogLevel::ERROR:   levelStr = "ERROR"; break;
        }

        std::time_t now = std::time(nullptr);
        char timeBuf[100];
        std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

        std::ostringstream oss;
        oss << "[" << timeBuf << "] [" << levelStr << "] " << name_ << ": " << message;
        *outputStream_ << oss.str() << std::endl;
    }

    void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    void info(const std::string& message) { log(LogLevel::INFO, message); }
    void warning(const std::string& message) { log(LogLevel::WARNING, message); }
    void error(const std::string& message) { log(LogLevel::ERROR, message); }

private:
    std::string name_;
    LogLevel minLevel_;
    std::ostream* outputStream_;
    std::unique_ptr<std::ofstream> fileStream_;
};