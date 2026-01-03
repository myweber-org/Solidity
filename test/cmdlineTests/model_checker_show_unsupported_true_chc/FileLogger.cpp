
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <queue>
#include <thread>

namespace fs = std::filesystem;

class ThreadSafeLogger {
private:
    std::ofstream logFile;
    std::mutex logMutex;
    std::string currentDate;
    std::string logDirectory;
    size_t maxFiles;
    std::queue<std::string> logQueue;
    std::thread writerThread;
    bool stopFlag;

    std::string getDateString() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d");
        return ss.str();
    }

    void rotateLogFile() {
        std::string today = getDateString();
        if (today != currentDate) {
            if (logFile.is_open()) {
                logFile.close();
            }
            
            fs::create_directories(logDirectory);
            
            std::string filename = logDirectory + "/app_" + today + ".log";
            logFile.open(filename, std::ios::app);
            
            if (!logFile.is_open()) {
                throw std::runtime_error("Cannot open log file: " + filename);
            }
            
            currentDate = today;
            cleanupOldFiles();
        }
    }

    void cleanupOldFiles() {
        try {
            std::vector<fs::directory_entry> files;
            for (const auto& entry : fs::directory_iterator(logDirectory)) {
                if (entry.is_regular_file() && 
                    entry.path().extension() == ".log") {
                    files.push_back(entry);
                }
            }
            
            if (files.size() > maxFiles) {
                std::sort(files.begin(), files.end(),
                    [](const fs::directory_entry& a, const fs::directory_entry& b) {
                        return fs::last_write_time(a) < fs::last_write_time(b);
                    });
                
                for (size_t i = 0; i < files.size() - maxFiles; ++i) {
                    fs::remove(files[i].path());
                }
            }
        } catch (const fs::filesystem_error&) {
            // Silently ignore filesystem errors during cleanup
        }
    }

    void writerLoop() {
        while (!stopFlag || !logQueue.empty()) {
            std::string message;
            bool hasMessage = false;
            
            {
                std::lock_guard<std::mutex> lock(logMutex);
                if (!logQueue.empty()) {
                    message = logQueue.front();
                    logQueue.pop();
                    hasMessage = true;
                }
            }
            
            if (hasMessage) {
                rotateLogFile();
                logFile << message << std::endl;
                logFile.flush();
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }

public:
    ThreadSafeLogger(const std::string& directory = "./logs", size_t maxLogFiles = 30)
        : logDirectory(directory), maxFiles(maxLogFiles), stopFlag(false) {
        currentDate = getDateString();
        rotateLogFile();
        writerThread = std::thread(&ThreadSafeLogger::writerLoop, this);
    }

    ~ThreadSafeLogger() {
        stopFlag = true;
        if (writerThread.joinable()) {
            writerThread.join();
        }
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(const std::string& level, const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        ss << " [" << level << "] " << message;
        
        std::lock_guard<std::mutex> lock(logMutex);
        logQueue.push(ss.str());
    }

    void info(const std::string& message) {
        log("INFO", message);
    }

    void warning(const std::string& message) {
        log("WARN", message);
    }

    void error(const std::string& message) {
        log("ERROR", message);
    }

    void debug(const std::string& message) {
        log("DEBUG", message);
    }
};