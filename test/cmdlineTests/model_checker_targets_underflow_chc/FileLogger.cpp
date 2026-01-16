
#include <fstream>
#include <filesystem>
#include <chrono>
#include <zlib.h>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace fs = std::filesystem;

class FileLogger {
public:
    FileLogger(const std::string& basePath, size_t maxFileSize = 1048576, int maxBackups = 5)
        : basePath_(basePath), maxFileSize_(maxFileSize), maxBackups_(maxBackups), running_(true) {
        workerThread_ = std::thread(&FileLogger::processQueue, this);
    }

    ~FileLogger() {
        running_ = false;
        cv_.notify_one();
        if (workerThread_.joinable()) {
            workerThread_.join();
        }
        flushQueue();
    }

    void log(const std::string& message) {
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            messageQueue_.push(message);
        }
        cv_.notify_one();
    }

private:
    void processQueue() {
        while (running_ || !messageQueue_.empty()) {
            std::unique_lock<std::mutex> lock(queueMutex_);
            cv_.wait(lock, [this] { return !messageQueue_.empty() || !running_; });

            if (!messageQueue_.empty()) {
                std::string msg = messageQueue_.front();
                messageQueue_.pop();
                lock.unlock();

                writeToFile(msg);
                checkRotation();
            }
        }
    }

    void writeToFile(const std::string& message) {
        std::ofstream file(currentPath(), std::ios::app);
        if (file.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            file << std::ctime(&time) << ": " << message << std::endl;
        }
    }

    void checkRotation() {
        if (fs::file_size(currentPath()) > maxFileSize_) {
            rotateFiles();
            compressOldest();
        }
    }

    void rotateFiles() {
        for (int i = maxBackups_ - 1; i > 0; --i) {
            fs::path oldFile = backupPath(i);
            fs::path newFile = backupPath(i + 1);
            if (fs::exists(oldFile)) {
                fs::rename(oldFile, newFile);
            }
        }
        fs::rename(currentPath(), backupPath(1));
    }

    void compressOldest() {
        fs::path oldest = backupPath(maxBackups_);
        if (fs::exists(oldest)) {
            compressFile(oldest);
        }
    }

    void compressFile(const fs::path& filePath) {
        std::ifstream input(filePath, std::ios::binary);
        if (!input) return;

        std::vector<char> buffer(std::istreambuf_iterator<char>(input), {});
        input.close();

        uLongf compressedSize = compressBound(buffer.size());
        std::vector<Bytef> compressed(compressedSize);

        if (compress(compressed.data(), &compressedSize,
                    reinterpret_cast<Bytef*>(buffer.data()), buffer.size()) == Z_OK) {
            fs::path compressedPath = filePath.string() + ".gz";
            std::ofstream output(compressedPath, std::ios::binary);
            output.write(reinterpret_cast<char*>(compressed.data()), compressedSize);
            fs::remove(filePath);
        }
    }

    fs::path currentPath() const {
        return basePath_ / "application.log";
    }

    fs::path backupPath(int index) const {
        return basePath_ / ("application.log." + std::to_string(index));
    }

    void flushQueue() {
        std::lock_guard<std::mutex> lock(queueMutex_);
        while (!messageQueue_.empty()) {
            writeToFile(messageQueue_.front());
            messageQueue_.pop();
        }
    }

    fs::path basePath_;
    size_t maxFileSize_;
    int maxBackups_;
    std::queue<std::string> messageQueue_;
    std::mutex queueMutex_;
    std::condition_variable cv_;
    std::thread workerThread_;
    std::atomic<bool> running_;
};