
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
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher() : running_(false) {}

    ~FileSystemWatcher() {
        stop();
    }

    void addWatchPath(const fs::path& path, Callback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (fs::exists(path) && fs::is_directory(path)) {
            watch_paths_[path] = {callback, getDirectoryState(path)};
        }
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
        std::unordered_map<std::string, fs::file_time_type> file_timestamps;
    };

    std::unordered_map<fs::path, WatchInfo> watch_paths_;
    std::atomic<bool> running_;
    std::thread monitor_thread_;
    std::mutex mutex_;

    std::unordered_map<std::string, fs::file_time_type> getDirectoryState(const fs::path& dir) {
        std::unordered_map<std::string, fs::file_time_type> state;
        
        for (const auto& entry : fs::recursive_directory_iterator(dir)) {
            if (fs::is_regular_file(entry.status())) {
                state[entry.path().string()] = fs::last_write_time(entry);
            }
        }
        
        return state;
    }

    void monitorLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            std::lock_guard<std::mutex> lock(mutex_);
            
            for (auto& [path, watch_info] : watch_paths_) {
                auto current_state = getDirectoryState(path);
                auto& previous_state = watch_info.file_timestamps;
                
                for (const auto& [file_path, current_time] : current_state) {
                    auto it = previous_state.find(file_path);
                    if (it == previous_state.end()) {
                        watch_info.callback(file_path, "created");
                    } else if (it->second != current_time) {
                        watch_info.callback(file_path, "modified");
                    }
                }
                
                for (const auto& [file_path, _] : previous_state) {
                    if (current_state.find(file_path) == current_state.end()) {
                        watch_info.callback(file_path, "deleted");
                    }
                }
                
                previous_state = std::move(current_state);
            }
        }
    }
};

void exampleCallback(const fs::path& path, const std::string& action) {
    std::cout << "File: " << path << " Action: " << action << std::endl;
}

int main() {
    FileSystemWatcher watcher;
    
    watcher.addWatchPath("./test_directory", exampleCallback);
    
    std::cout << "Starting file system watcher. Monitoring ./test_directory" << std::endl;
    std::cout << "Press Enter to stop..." << std::endl;
    
    watcher.start();
    
    std::cin.get();
    
    watcher.stop();
    
    return 0;
}