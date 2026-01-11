#include <iostream>
#include <chrono>
#include <thread>
#include <unordered_set>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind/bind.hpp>

namespace fs = boost::filesystem;
namespace asio = boost::asio;

class FileSystemWatcher {
public:
    FileSystemWatcher(asio::io_context& io, const std::string& path)
        : timer_(io), watch_path_(path), running_(false) {
        if (!fs::exists(watch_path_) || !fs::is_directory(watch_path_)) {
            throw std::runtime_error("Invalid directory path");
        }
        scan_existing_files();
    }

    void start() {
        running_ = true;
        schedule_next_scan();
    }

    void stop() {
        running_ = false;
        timer_.cancel();
    }

    void set_change_callback(std::function<void(const std::string&, bool)> cb) {
        change_callback_ = cb;
    }

private:
    void scan_existing_files() {
        for (fs::recursive_directory_iterator it(watch_path_), end; it != end; ++it) {
            if (fs::is_regular_file(it->status())) {
                std::string file_path = it->path().string();
                file_timestamps_[file_path] = fs::last_write_time(it->path());
            }
        }
    }

    void schedule_next_scan() {
        if (!running_) return;
        
        timer_.expires_after(std::chrono::seconds(2));
        timer_.async_wait([this](const boost::system::error_code& ec) {
            if (!ec && running_) {
                perform_scan();
                schedule_next_scan();
            }
        });
    }

    void perform_scan() {
        std::unordered_set<std::string> current_files;
        
        for (fs::recursive_directory_iterator it(watch_path_), end; it != end; ++it) {
            if (fs::is_regular_file(it->status())) {
                std::string file_path = it->path().string();
                current_files.insert(file_path);
                
                auto last_write = fs::last_write_time(it->path());
                auto existing = file_timestamps_.find(file_path);
                
                if (existing == file_timestamps_.end()) {
                    file_timestamps_[file_path] = last_write;
                    if (change_callback_) {
                        change_callback_(file_path, true);
                    }
                } else if (existing->second != last_write) {
                    existing->second = last_write;
                    if (change_callback_) {
                        change_callback_(file_path, false);
                    }
                }
            }
        }
        
        for (auto it = file_timestamps_.begin(); it != file_timestamps_.end();) {
            if (current_files.find(it->first) == current_files.end()) {
                if (change_callback_) {
                    change_callback_(it->first, false);
                }
                it = file_timestamps_.erase(it);
            } else {
                ++it;
            }
        }
    }

    asio::steady_timer timer_;
    std::string watch_path_;
    std::unordered_map<std::string, std::time_t> file_timestamps_;
    std::function<void(const std::string&, bool)> change_callback_;
    bool running_;
};

int main() {
    try {
        asio::io_context io;
        FileSystemWatcher watcher(io, ".");
        
        watcher.set_change_callback([](const std::string& path, bool is_new) {
            std::cout << (is_new ? "[NEW] " : "[MODIFIED] ") << path << std::endl;
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