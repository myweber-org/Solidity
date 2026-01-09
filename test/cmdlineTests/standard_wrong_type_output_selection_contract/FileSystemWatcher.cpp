
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <mutex>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::error_code&)>;

    FileSystemWatcher() : running_(false) {}

    ~FileSystemWatcher() {
        stop();
    }

    void addWatchPath(const fs::path& path, Callback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (fs::exists(path) && fs::is_directory(path)) {
            watch_paths_[path] = {callback, fs::last_write_time(path)};
        }
    }

    void removeWatchPath(const fs::path& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        watch_paths_.erase(path);
    }

    void start() {
        if (running_) return;
        running_ = true;
        monitor_thread_ = std::thread(&FileSystemWatcher::monitorLoop, this);
    }

    void stop() {
        running_ = false;
        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
    }

private:
    struct WatchInfo {
        Callback callback;
        fs::file_time_type last_write_time;
    };

    std::unordered_map<fs::path, WatchInfo> watch_paths_;
    std::atomic<bool> running_;
    std::thread monitor_thread_;
    std::mutex mutex_;

    void monitorLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto& [path, info] : watch_paths_) {
                std::error_code ec;
                if (!fs::exists(path, ec)) {
                    info.callback(path, ec);
                    continue;
                }

                auto current_time = fs::last_write_time(path, ec);
                if (ec) {
                    info.callback(path, ec);
                    continue;
                }

                if (current_time != info.last_write_time) {
                    info.last_write_time = current_time;
                    info.callback(path, std::error_code{});
                }
            }
        }
    }
};

void exampleCallback(const fs::path& path, const std::error_code& ec) {
    if (ec) {
        std::cerr << "Error watching " << path << ": " << ec.message() << std::endl;
    } else {
        std::cout << "File modified: " << path << std::endl;
    }
}

int main() {
    FileSystemWatcher watcher;
    
    watcher.addWatchPath("/tmp/monitor", exampleCallback);
    watcher.addWatchPath("/var/log", exampleCallback);
    
    watcher.start();
    
    std::cout << "File system watcher started. Monitoring for 30 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(30));
    
    watcher.stop();
    std::cout << "File system watcher stopped." << std::endl;
    
    return 0;
}