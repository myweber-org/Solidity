
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind/bind.hpp>

namespace fs = boost::filesystem;
namespace asio = boost::asio;

class FileSystemWatcher {
public:
    FileSystemWatcher(asio::io_context& io, const std::string& path)
        : io_context_(io), path_(path), timer_(io), running_(false) {
        if (!fs::exists(path_) || !fs::is_directory(path_)) {
            throw std::runtime_error("Invalid directory path");
        }
        last_write_time_ = fs::last_write_time(path_);
    }

    void start() {
        running_ = true;
        schedule_check();
    }

    void stop() {
        running_ = false;
        timer_.cancel();
    }

    void set_change_callback(std::function<void(const std::string&)> callback) {
        change_callback_ = callback;
    }

private:
    void schedule_check() {
        if (!running_) return;
        
        timer_.expires_after(std::chrono::seconds(1));
        timer_.async_wait([this](const boost::system::error_code& ec) {
            if (!ec && running_) {
                check_for_changes();
                schedule_check();
            }
        });
    }

    void check_for_changes() {
        try {
            std::time_t current_time = fs::last_write_time(path_);
            if (current_time != last_write_time_) {
                last_write_time_ = current_time;
                if (change_callback_) {
                    change_callback_(path_);
                }
                std::cout << "Directory modified: " << path_ << std::endl;
            }

            for (fs::directory_iterator it(path_); it != fs::directory_iterator(); ++it) {
                if (fs::is_regular_file(it->path())) {
                    std::time_t file_time = fs::last_write_time(it->path());
                    auto iter = file_times_.find(it->path().string());
                    
                    if (iter == file_times_.end()) {
                        file_times_[it->path().string()] = file_time;
                        std::cout << "New file detected: " << it->path() << std::endl;
                    } else if (iter->second != file_time) {
                        iter->second = file_time;
                        std::cout << "File modified: " << it->path() << std::endl;
                    }
                }
            }

            auto it = file_times_.begin();
            while (it != file_times_.end()) {
                if (!fs::exists(it->first)) {
                    std::cout << "File deleted: " << it->first << std::endl;
                    it = file_times_.erase(it);
                } else {
                    ++it;
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
    }

    asio::io_context& io_context_;
    std::string path_;
    asio::steady_timer timer_;
    std::atomic<bool> running_;
    std::time_t last_write_time_;
    std::unordered_map<std::string, std::time_t> file_times_;
    std::function<void(const std::string&)> change_callback_;
};

void handle_directory_change(const std::string& path) {
    std::cout << "Change detected in directory: " << path << std::endl;
}

int main() {
    try {
        asio::io_context io;
        FileSystemWatcher watcher(io, ".");
        
        watcher.set_change_callback(handle_directory_change);
        watcher.start();
        
        std::thread io_thread([&io]() { io.run(); });
        
        std::cout << "Watching current directory. Press Enter to stop..." << std::endl;
        std::cin.get();
        
        watcher.stop();
        io.stop();
        io_thread.join();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}