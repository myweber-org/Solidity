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
        } else {
            std::cerr << "Invalid or non-existent directory: " << path << std::endl;
        }
    }

    void setCallback(Callback cb) {
        callback_ = std::move(cb);
    }

    void start() {
        running_ = true;
        snapshot();
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
    void snapshot() {
        std::lock_guard<std::mutex> lock(mutex_);
        file_snapshots_.clear();
        for (const auto& dir : watch_paths_) {
            for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                if (entry.is_regular_file()) {
                    auto last_write = fs::last_write_time(entry.path());
                    file_snapshots_[entry.path()] = last_write;
                }
            }
        }
    }

    void monitor() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            checkForChanges();
        }
    }

    void checkForChanges() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::unordered_map<fs::path, fs::file_time_type> current_snapshot;

        for (const auto& dir : watch_paths_) {
            if (!fs::exists(dir)) continue;
            for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                if (entry.is_regular_file()) {
                    auto path = entry.path();
                    auto last_write = fs::last_write_time(path);
                    current_snapshot[path] = last_write;

                    auto it = file_snapshots_.find(path);
                    if (it == file_snapshots_.end()) {
                        if (callback_) callback_(path, "created");
                    } else if (it->second != last_write) {
                        if (callback_) callback_(path, "modified");
                    }
                }
            }
        }

        for (const auto& [path, _] : file_snapshots_) {
            if (current_snapshot.find(path) == current_snapshot.end()) {
                if (callback_) callback_(path, "deleted");
            }
        }

        file_snapshots_.swap(current_snapshot);
    }

    std::vector<fs::path> watch_paths_;
    std::unordered_map<fs::path, fs::file_time_type> file_snapshots_;
    Callback callback_;
    std::thread monitor_thread_;
    std::atomic<bool> running_;
    std::mutex mutex_;
};

int main() {
    FileSystemWatcher watcher;
    
    watcher.setCallback([](const fs::path& path, const std::string& action) {
        std::cout << "File " << path << " was " << action << " at " 
                  << std::chrono::system_clock::now() << std::endl;
    });

    watcher.addWatchPath(".");
    watcher.start();

    std::cout << "File system watcher started. Press Enter to stop..." << std::endl;
    std::cin.get();

    watcher.stop();
    return 0;
}