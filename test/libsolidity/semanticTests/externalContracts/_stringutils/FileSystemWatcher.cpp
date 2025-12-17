
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

namespace fs = boost::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(boost::asio::io_context& io, const std::string& path)
        : timer_(io), watch_path_(path), running_(false) {
        if (!fs::exists(watch_path_) || !fs::is_directory(watch_path_)) {
            throw std::runtime_error("Invalid directory path");
        }
    }

    void start() {
        if (running_) return;
        running_ = true;
        last_check_time_ = fs::last_write_time(watch_path_);
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
            std::time_t current_time = fs::last_write_time(watch_path_);
            if (current_time != last_check_time_) {
                last_check_time_ = current_time;
                if (change_callback_) {
                    change_callback_(watch_path_.string());
                }
                std::cout << "Directory modified: " << watch_path_ << std::endl;
            }

            for (fs::recursive_directory_iterator it(watch_path_), end; it != end; ++it) {
                if (fs::is_regular_file(*it)) {
                    std::time_t file_time = fs::last_write_time(*it);
                    auto file_path = it->path().string();
                    
                    auto it_cache = file_cache_.find(file_path);
                    if (it_cache == file_cache_.end()) {
                        file_cache_[file_path] = file_time;
                        if (change_callback_) {
                            change_callback_(file_path);
                        }
                        std::cout << "New file detected: " << file_path << std::endl;
                    } else if (it_cache->second != file_time) {
                        it_cache->second = file_time;
                        if (change_callback_) {
                            change_callback_(file_path);
                        }
                        std::cout << "File modified: " << file_path << std::endl;
                    }
                }
            }

            auto it = file_cache_.begin();
            while (it != file_cache_.end()) {
                if (!fs::exists(it->first)) {
                    if (change_callback_) {
                        change_callback_(it->first);
                    }
                    std::cout << "File deleted: " << it->first << std::endl;
                    it = file_cache_.erase(it);
                } else {
                    ++it;
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error checking directory: " << e.what() << std::endl;
        }
    }

    boost::asio::steady_timer timer_;
    fs::path watch_path_;
    std::unordered_map<std::string, std::time_t> file_cache_;
    std::time_t last_check_time_;
    bool running_;
    std::function<void(const std::string&)> change_callback_;
};

int main() {
    try {
        boost::asio::io_context io;
        FileSystemWatcher watcher(io, ".");
        
        watcher.set_change_callback([](const std::string& path) {
            std::cout << "Change detected in: " << path << std::endl;
        });
        
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