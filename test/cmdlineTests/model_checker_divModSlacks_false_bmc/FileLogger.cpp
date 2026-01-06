
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <queue>
#include <thread>
#include <condition_variable>

namespace logging {

class FileLogger {
public:
    FileLogger(const std::string& base_path, size_t max_file_size = 1048576, size_t max_files = 10)
        : base_path_(base_path), max_file_size_(max_file_size), max_files_(max_files),
          current_size_(0), running_(true) {
        std::filesystem::create_directories(std::filesystem::path(base_path).parent_path());
        rotate_if_needed();
        worker_thread_ = std::thread(&FileLogger::process_queue, this);
    }

    ~FileLogger() {
        running_ = false;
        cv_.notify_one();
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
        flush_remaining();
    }

    void log(const std::string& message, const std::string& level = "INFO") {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        ss << " [" << level << "] " << message << "\n";

        std::lock_guard<std::mutex> lock(queue_mutex_);
        message_queue_.push(ss.str());
        cv_.notify_one();
    }

private:
    void process_queue() {
        while (running_ || !message_queue_.empty()) {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            cv_.wait(lock, [this] { return !message_queue_.empty() || !running_; });

            std::queue<std::string> local_queue;
            std::swap(local_queue, message_queue_);
            lock.unlock();

            while (!local_queue.empty()) {
                write_to_file(local_queue.front());
                local_queue.pop();
            }
        }
    }

    void write_to_file(const std::string& message) {
        std::lock_guard<std::mutex> lock(file_mutex_);
        
        if (current_size_ + message.size() > max_file_size_) {
            rotate();
        }

        std::ofstream file(base_path_, std::ios::app);
        if (file.is_open()) {
            file << message;
            file.flush();
            current_size_ += message.size();
        }
    }

    void rotate_if_needed() {
        std::lock_guard<std::mutex> lock(file_mutex_);
        if (std::filesystem::exists(base_path_)) {
            current_size_ = std::filesystem::file_size(base_path_);
            if (current_size_ >= max_file_size_) {
                rotate();
            }
        } else {
            current_size_ = 0;
        }
    }

    void rotate() {
        for (int i = max_files_ - 1; i > 0; --i) {
            std::string old_file = base_path_ + "." + std::to_string(i);
            std::string new_file = base_path_ + "." + std::to_string(i + 1);
            
            if (std::filesystem::exists(old_file)) {
                if (i == max_files_ - 1) {
                    std::filesystem::remove(old_file);
                } else {
                    std::filesystem::rename(old_file, new_file);
                }
            }
        }
        
        if (std::filesystem::exists(base_path_)) {
            std::filesystem::rename(base_path_, base_path_ + ".1");
        }
        
        current_size_ = 0;
    }

    void flush_remaining() {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        while (!message_queue_.empty()) {
            write_to_file(message_queue_.front());
            message_queue_.pop();
        }
    }

    std::string base_path_;
    size_t max_file_size_;
    size_t max_files_;
    size_t current_size_;
    
    std::mutex file_mutex_;
    std::mutex queue_mutex_;
    std::condition_variable cv_;
    std::queue<std::string> message_queue_;
    
    std::thread worker_thread_;
    std::atomic<bool> running_;
};

} // namespace logging