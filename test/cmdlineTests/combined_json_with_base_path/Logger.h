
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
    ERROR
};

class Logger {
public:
    static Logger& getInstance();
    
    void setLogFile(const std::string& filename);
    void setMinLevel(LogLevel level);
    
    void log(LogLevel level, const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    
private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    std::string getLevelString(LogLevel level) const;
    std::string getCurrentTimestamp() const;
    
    std::ofstream logFile;
    LogLevel minLevel;
    std::mutex logMutex;
    bool fileOpened;
};

#endif