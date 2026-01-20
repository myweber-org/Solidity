#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <memory>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

class Logger {
public:
    static Logger& getInstance();
    
    void setLogLevel(LogLevel level);
    void setLogFile(const std::string& filename);
    
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void critical(const std::string& message);
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
private:
    Logger();
    ~Logger();
    
    void log(LogLevel level, const std::string& message);
    std::string levelToString(LogLevel level);
    std::string getCurrentTimestamp();
    
    LogLevel currentLevel;
    std::unique_ptr<std::ofstream> logFile;
    std::mutex logMutex;
    bool useFileOutput;
};

#endif