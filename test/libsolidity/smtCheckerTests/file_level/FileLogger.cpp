
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <queue>
#include <condition_variable>

namespace logging {

class FileLogger {
public:
    FileLogger(const std::string& basePath, size_t maxFileSize = 1048576, int maxFiles = 5)
        : basePath_(basePath), maxFileSize_(maxFileSize), maxFiles_(maxFiles), running_(true) {
        workerThread_ = std::thread(&FileLogger::processQueue, this);
    }

    ~FileLogger() {
        running_ = false;
        cv_.notify_all();
        if (workerThread_.joinable()) {
            workerThread_.join();
        }
    }

    void log(const std::string& message, const std::string& level = "INFO") {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        ss << " [" << level << "] " << message << "\n";

        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            logQueue_.push(ss.str());
        }
        cv_.notify_one();
    }

private:
    void processQueue() {
        while (running_ || !logQueue_.empty()) {
            std::string message;
            {
                std::unique_lock<std::mutex> lock(queueMutex_);
                cv_.wait(lock, [this] { return !logQueue_.empty() || !running_; });
                
                if (!logQueue_.empty()) {
                    message = logQueue_.front();
                    logQueue_.pop();
                }
            }

            if (!message.empty()) {
                writeToFile(message);
            }
        }
    }

    void writeToFile(const std::string& message) {
        std::lock_guard<std::mutex> lock(fileMutex_);
        
        std::ofstream file(currentFilePath(), std::ios::app);
        if (file.is_open()) {
            file << message;
            file.close();
            
            if (std::filesystem::file_size(currentFilePath()) > maxFileSize_) {
                rotateFiles();
            }
        }
    }

    std::string currentFilePath() const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << basePath_ << "_" << std::put_time(std::localtime(&time), "%Y%m%d") << ".log";
        return ss.str();
    }

    void rotateFiles() {
        std::vector<std::string> existingFiles;
        for (const auto& entry : std::filesystem::directory_iterator(".")) {
            if (entry.path().extension() == ".log" && 
                entry.path().string().find(basePath_) == 0) {
                existingFiles.push_back(entry.path().string());
            }
        }
        
        std::sort(existingFiles.begin(), existingFiles.end());
        
        while (existingFiles.size() >= static_cast<size_t>(maxFiles_)) {
            std::filesystem::remove(existingFiles.front());
            existingFiles.erase(existingFiles.begin());
        }
        
        for (int i = existingFiles.size() - 1; i >= 0; --i) {
            std::string newName = basePath_ + "_" + std::to_string(i + 1) + ".log";
            std::filesystem::rename(existingFiles[i], newName);
        }
    }

    std::string basePath_;
    size_t maxFileSize_;
    int maxFiles_;
    
    std::queue<std::string> logQueue_;
    std::mutex queueMutex_;
    std::mutex fileMutex_;
    std::condition_variable cv_;
    
    std::thread workerThread_;
    std::atomic<bool> running_;
};

} // namespace logging