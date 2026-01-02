
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <filesystem>

class FileLogger {
public:
    explicit FileLogger(const std::string& basePath, size_t maxSize = 1048576, int maxFiles = 5)
        : basePath_(basePath), maxSize_(maxSize), maxFiles_(maxFiles), currentSize_(0) {
        rotateIfNeeded();
    }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        std::ofstream file(basePath_, std::ios::app);
        if (file.is_open()) {
            file << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
                 << " | " << message << std::endl;
            currentSize_ += message.length() + 30;
            rotateIfNeeded();
        }
    }

private:
    void rotateIfNeeded() {
        if (currentSize_ < maxSize_) return;
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (int i = maxFiles_ - 1; i > 0; --i) {
            std::string oldName = basePath_ + "." + std::to_string(i);
            std::string newName = basePath_ + "." + std::to_string(i + 1);
            
            if (std::filesystem::exists(oldName)) {
                if (std::filesystem::exists(newName)) {
                    std::filesystem::remove(newName);
                }
                std::filesystem::rename(oldName, newName);
            }
        }
        
        if (std::filesystem::exists(basePath_)) {
            std::filesystem::rename(basePath_, basePath_ + ".1");
        }
        
        currentSize_ = 0;
    }

    std::string basePath_;
    size_t maxSize_;
    int maxFiles_;
    size_t currentSize_;
    std::mutex mutex_;
};