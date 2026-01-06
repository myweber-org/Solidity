
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <filesystem>

class FileLogger {
public:
    explicit FileLogger(const std::string& base_path, size_t max_size = 1048576, int max_files = 5)
        : base_path_(base_path), max_size_(max_size), max_files_(max_files), current_size_(0) {
        rotate_if_needed();
        open_current();
    }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        file_ << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        file_ << " | " << message << std::endl;
        
        current_size_ += message.size() + 50;
        if (current_size_ >= max_size_) {
            rotate();
        }
    }

private:
    void open_current() {
        file_.open(base_path_, std::ios::app);
        if (file_) {
            file_.seekp(0, std::ios::end);
            current_size_ = file_.tellp();
        }
    }

    void rotate_if_needed() {
        if (std::filesystem::exists(base_path_)) {
            std::error_code ec;
            auto size = std::filesystem::file_size(base_path_, ec);
            if (!ec && size >= max_size_) {
                rotate();
            }
        }
    }

    void rotate() {
        file_.close();
        
        for (int i = max_files_ - 1; i > 0; --i) {
            std::string old_name = base_path_ + "." + std::to_string(i);
            std::string new_name = base_path_ + "." + std::to_string(i + 1);
            
            if (std::filesystem::exists(old_name)) {
                std::filesystem::rename(old_name, new_name);
            }
        }
        
        std::filesystem::rename(base_path_, base_path_ + ".1");
        open_current();
        current_size_ = 0;
    }

    std::string base_path_;
    size_t max_size_;
    int max_files_;
    std::ofstream file_;
    std::mutex mutex_;
    size_t current_size_;
};