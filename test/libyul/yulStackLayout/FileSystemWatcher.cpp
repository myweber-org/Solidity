
#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <unordered_map>
#include <functional>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher() : running_(false) {}

    ~FileSystemWatcher() {
        stop();
    }

    void addWatchPath(const fs::path& path) {
        if (fs::exists(path) && fs::is_directory(path)) {
            std::lock_guard<std::mutex> lock(mutex_);
            watch_paths_[path] = getLastWriteTime(path);
        }
    }

    void setCallback(Callback callback) {
        callback_ = std::move(callback);
    }

    void start() {
        if (running_) return;
        
        running_ = true;
        watcher_thread_ = std::thread(&FileSystemWatcher::watchLoop, this);
    }

    void stop() {
        running_ = false;
        cv_.notify_all();
        if (watcher_thread_.joinable()) {
            watcher_thread_.join();
        }
    }

private:
    std::time_t getLastWriteTime(const fs::path& path) {
        auto ftime = fs::last_write_time(path);
        return decltype(ftime)::clock::to_time_t(ftime);
    }

    void watchLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto& [path, last_time] : watch_paths_) {
                if (!fs::exists(path)) continue;
                
                auto current_time = getLastWriteTime(path);
                if (current_time != last_time) {
                    last_time = current_time;
                    if (callback_) {
                        callback_(path, "modified");
                    }
                }
            }
        }
    }

    std::atomic<bool> running_;
    std::thread watcher_thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::unordered_map<fs::path, std::time_t> watch_paths_;
    Callback callback_;
};

void exampleCallback(const fs::path& path, const std::string& action) {
    std::cout << "File " << path << " was " << action << " at " 
              << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) 
              << std::endl;
}

int main() {
    FileSystemWatcher watcher;
    watcher.setCallback(exampleCallback);
    watcher.addWatchPath(".");
    
    std::cout << "Starting file system watcher. Monitoring current directory..." << std::endl;
    watcher.start();
    
    std::this_thread::sleep_for(std::chrono::seconds(30));
    
    watcher.stop();
    std::cout << "File system watcher stopped." << std::endl;
    
    return 0;
}