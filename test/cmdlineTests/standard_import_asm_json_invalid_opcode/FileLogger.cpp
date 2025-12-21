
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
    explicit FileLogger(const std::string& base_path, 
                       size_t max_file_size = 1048576, 
                       int max_files = 10)
        : base_path_(base_path)
        , max_file_size_(max_file_size)
        , max_files_(max_files)
        , current_size_(0)
        , running_(true)
        , worker_thread_(&FileLogger::process_queue, this) {
        
        std::filesystem::create_directories(std::filesystem::path(base_path).parent_path());
        rotate_if_needed();
    }

    ~FileLogger() {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            running_ = false;
        }
        queue_cv_.notify_all();
        worker_thread_.join();
        
        while (!log_queue_.empty()) {
            write_entry(log_queue_.front());
            log_queue_.pop();
        }
    }

    void log(const std::string& level, const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S")
           << '.' << std::setfill('0') << std::setw(3) << ms.count()
           << " [" << level << "] " << message << "\n";

        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            log_queue_.push(ss.str());
        }
        queue_cv_.notify_one();
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

private:
    void process_queue() {
        while (true) {
            std::string entry;
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                queue_cv_.wait(lock, [this]() { 
                    return !log_queue_.empty() || !running_; 
                });

                if (!running_ && log_queue_.empty()) {
                    break;
                }

                if (!log_queue_.empty()) {
                    entry = log_queue_.front();
                    log_queue_.pop();
                }
            }

            if (!entry.empty()) {
                write_entry(entry);
            }
        }
    }

    void write_entry(const std::string& entry) {
        std::lock_guard<std::mutex> lock(file_mutex_);
        
        if (current_size_ + entry.size() > max_file_size_) {
            rotate();
        }

        std::ofstream file(current_file_, std::ios::app);
        if (file.is_open()) {
            file << entry;
            file.flush();
            current_size_ += entry.size();
        }
    }

    void rotate_if_needed() {
        std::lock_guard<std::mutex> lock(file_mutex_);
        
        current_file_ = get_current_filename();
        if (std::filesystem::exists(current_file_)) {
            current_size_ = std::filesystem::file_size(current_file_);
            if (current_size_ >= max_file_size_) {
                rotate();
            }
        } else {
            current_size_ = 0;
        }
    }

    void rotate() {
        std::filesystem::path base_path(base_path_);
        std::string base_name = base_path.stem().string();
        std::string extension = base_path.extension().string();
        std::string directory = base_path.parent_path().string();

        for (int i = max_files_ - 1; i > 0; --i) {
            std::string old_name = directory + "/" + base_name + 
                                  (i > 1 ? "." + std::to_string(i - 1) : "") + extension;
            std::string new_name = directory + "/" + base_name + "." + 
                                  std::to_string(i) + extension;

            if (std::filesystem::exists(old_name)) {
                std::filesystem::rename(old_name, new_name);
            }
        }

        if (std::filesystem::exists(current_file_)) {
            std::filesystem::rename(current_file_, 
                directory + "/" + base_name + ".1" + extension);
        }

        current_size_ = 0;
    }

    std::string get_current_filename() const {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << base_path_ << "." 
           << std::put_time(std::localtime(&time_t), "%Y%m%d");
        return ss.str();
    }

    std::string base_path_;
    size_t max_file_size_;
    int max_files_;
    std::string current_file_;
    size_t current_size_;
    
    std::mutex file_mutex_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::queue<std::string> log_queue_;
    bool running_;
    std::thread worker_thread_;
};

} // namespace logging