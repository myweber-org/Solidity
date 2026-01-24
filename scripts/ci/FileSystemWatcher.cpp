
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

    void addWatchPath(const fs::path& path, Callback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (fs::exists(path) && fs::is_directory(path)) {
            watch_paths_[path] = {callback, fs::last_write_time(path)};
        }
    }

    void start() {
        running_ = true;
        monitor_thread_ = std::thread(&FileSystemWatcher::monitor, this);
    }

    void stop() {
        running_ = false;
        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

private:
    void monitor() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto& [path, data] : watch_paths_) {
                try {
                    if (!fs::exists(path)) continue;

                    auto current_time = fs::last_write_time(path);
                    if (data.last_write_time != current_time) {
                        data.callback(path, "modified");
                        data.last_write_time = current_time;
                    }

                    for (const auto& entry : fs::recursive_directory_iterator(path)) {
                        auto entry_path = entry.path();
                        auto it = tracked_files_.find(entry_path);
                        
                        if (it == tracked_files_.end()) {
                            tracked_files_[entry_path] = fs::last_write_time(entry);
                            data.callback(entry_path, "created");
                        } else {
                            auto current_entry_time = fs::last_write_time(entry);
                            if (it->second != current_entry_time) {
                                data.callback(entry_path, "modified");
                                it->second = current_entry_time;
                            }
                        }
                    }

                    auto it = tracked_files_.begin();
                    while (it != tracked_files_.end()) {
                        if (!fs::exists(it->first)) {
                            data.callback(it->first, "deleted");
                            it = tracked_files_.erase(it);
                        } else {
                            ++it;
                        }
                    }
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Filesystem error: " << e.what() << std::endl;
                }
            }
        }
    }

    struct WatchData {
        Callback callback;
        fs::file_time_type last_write_time;
    };

    std::unordered_map<fs::path, WatchData> watch_paths_;
    std::unordered_map<fs::path, fs::file_time_type> tracked_files_;
    std::atomic<bool> running_;
    std::thread monitor_thread_;
    std::mutex mutex_;
};

void exampleCallback(const fs::path& path, const std::string& action) {
    std::cout << "File " << path << " was " << action << " at " 
              << std::chrono::system_clock::now().time_since_epoch().count() 
              << std::endl;
}

int main() {
    FileSystemWatcher watcher;
    
    watcher.addWatchPath("./logs", exampleCallback);
    watcher.addWatchPath("./config", exampleCallback);
    
    std::cout << "Starting file system watcher. Press Enter to stop." << std::endl;
    watcher.start();
    
    std::cin.get();
    watcher.stop();
    
    return 0;
}