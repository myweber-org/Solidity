
#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
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
    
    void setLogLevel(LogLevel level);
    void enableFileOutput(const std::string& filename);
    void disableFileOutput();
    
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
private:
    Logger();
    ~Logger();
    
    void log(LogLevel level, const std::string& message);
    std::string getTimestamp() const;
    std::string levelToString(LogLevel level) const;
    
    LogLevel currentLevel;
    std::unique_ptr<std::ofstream> fileStream;
    bool fileOutputEnabled;
};

#endif