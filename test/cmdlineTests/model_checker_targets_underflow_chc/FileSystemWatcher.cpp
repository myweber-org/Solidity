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

    void addWatchPath(const fs::path& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (fs::exists(path) && fs::is_directory(path)) {
            watch_paths_.push_back(fs::canonical(path));
            std::cout << "Watching directory: " << watch_paths_.back() << std::endl;
        }
    }

    void setCallback(Callback cb) {
        callback_ = std::move(cb);
    }

    void start() {
        if (running_) return;
        
        running_ = true;
        snapshotFiles();
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
    void snapshotFiles() {
        std::lock_guard<std::mutex> lock(mutex_);
        file_snapshots_.clear();
        
        for (const auto& path : watch_paths_) {
            for (const auto& entry : fs::recursive_directory_iterator(path)) {
                if (entry.is_regular_file()) {
                    auto last_write = fs::last_write_time(entry);
                    file_snapshots_[entry.path()] = last_write;
                }
            }
        }
    }

    void monitorLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            checkForChanges();
        }
    }

    void checkForChanges() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::unordered_map<fs::path, fs::file_time_type> current_snapshot;
        
        for (const auto& path : watch_paths_) {
            if (!fs::exists(path)) continue;
            
            for (const auto& entry : fs::recursive_directory_iterator(path)) {
                if (entry.is_regular_file()) {
                    current_snapshot[entry.path()] = fs::last_write_time(entry);
                }
            }
        }

        for (const auto& [path, current_time] : current_snapshot) {
            auto it = file_snapshots_.find(path);
            if (it == file_snapshots_.end()) {
                if (callback_) callback_(path, "created");
            } else if (it->second != current_time) {
                if (callback_) callback_(path, "modified");
            }
        }

        for (const auto& [path, old_time] : file_snapshots_) {
            if (current_snapshot.find(path) == current_snapshot.end()) {
                if (callback_) callback_(path, "deleted");
            }
        }

        file_snapshots_ = std::move(current_snapshot);
    }

    std::vector<fs::path> watch_paths_;
    std::unordered_map<fs::path, fs::file_time_type> file_snapshots_;
    Callback callback_;
    std::atomic<bool> running_;
    std::thread monitor_thread_;
    std::mutex mutex_;
};

int main() {
    FileSystemWatcher watcher;
    
    watcher.setCallback([](const fs::path& path, const std::string& action) {
        std::cout << "File " << path << " was " << action << std::endl;
    });

    watcher.addWatchPath(".");
    watcher.start();

    std::cout << "File system watcher started. Press Enter to stop..." << std::endl;
    std::cin.get();
    
    watcher.stop();
    return 0;
}