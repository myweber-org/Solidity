
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <iostream>
#include <functional>
#include <atomic>
#include <mutex>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher() : running_(false) {}

    void addWatchPath(const fs::path& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (fs::exists(path)) {
            watch_paths_.push_back(fs::canonical(path));
        }
    }

    void setEventCallback(Callback callback) {
        callback_ = std::move(callback);
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

    ~FileSystemWatcher() {
        stop();
    }

private:
    void monitorLoop() {
        std::unordered_map<fs::path, fs::file_time_type> file_timestamps;

        for (const auto& path : watch_paths_) {
            if (fs::is_regular_file(path)) {
                file_timestamps[path] = fs::last_write_time(path);
            } else if (fs::is_directory(path)) {
                for (const auto& entry : fs::recursive_directory_iterator(path)) {
                    if (fs::is_regular_file(entry.path())) {
                        file_timestamps[entry.path()] = fs::last_write_time(entry.path());
                    }
                }
            }
        }

        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            std::lock_guard<std::mutex> lock(mutex_);
            
            for (auto it = file_timestamps.begin(); it != file_timestamps.end();) {
                if (!fs::exists(it->first)) {
                    if (callback_) {
                        callback_(it->first, "deleted");
                    }
                    it = file_timestamps.erase(it);
                } else {
                    ++it;
                }
            }

            for (const auto& watch_path : watch_paths_) {
                if (fs::is_regular_file(watch_path)) {
                    checkFileChange(watch_path, file_timestamps);
                } else if (fs::is_directory(watch_path)) {
                    for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
                        if (fs::is_regular_file(entry.path())) {
                            checkFileChange(entry.path(), file_timestamps);
                        }
                    }
                }
            }
        }
    }

    void checkFileChange(const fs::path& file_path, 
                        std::unordered_map<fs::path, fs::file_time_type>& timestamps) {
        auto current_time = fs::last_write_time(file_path);
        
        if (timestamps.find(file_path) == timestamps.end()) {
            timestamps[file_path] = current_time;
            if (callback_) {
                callback_(file_path, "created");
            }
        } else if (timestamps[file_path] != current_time) {
            timestamps[file_path] = current_time;
            if (callback_) {
                callback_(file_path, "modified");
            }
        }
    }

    std::vector<fs::path> watch_paths_;
    Callback callback_;
    std::atomic<bool> running_;
    std::thread monitor_thread_;
    std::mutex mutex_;
};

int main() {
    FileSystemWatcher watcher;
    
    watcher.addWatchPath(".");
    
    watcher.setEventCallback([](const fs::path& path, const std::string& event) {
        std::cout << "File: " << path.string() << " Event: " << event << std::endl;
    });
    
    std::cout << "Starting filesystem watcher. Monitoring current directory..." << std::endl;
    std::cout << "Press Enter to stop..." << std::endl;
    
    watcher.start();
    
    std::cin.get();
    
    watcher.stop();
    
    return 0;
}