
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <unordered_map>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const fs::path& watch_path, std::chrono::milliseconds interval)
        : watch_path_(watch_path), interval_(interval), running_(false) {
        if (!fs::exists(watch_path_) || !fs::is_directory(watch_path_)) {
            throw std::runtime_error("Path does not exist or is not a directory");
        }
        snapshot_ = take_snapshot();
    }

    void start() {
        running_ = true;
        watcher_thread_ = std::thread(&FileSystemWatcher::watch_loop, this);
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            running_ = false;
        }
        cv_.notify_all();
        if (watcher_thread_.joinable()) {
            watcher_thread_.join();
        }
    }

    ~FileSystemWatcher() {
        if (running_) {
            stop();
        }
    }

private:
    using FileSnapshot = std::unordered_map<std::string, fs::file_time_type>;

    FileSnapshot take_snapshot() {
        FileSnapshot snapshot;
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (fs::is_regular_file(entry.status())) {
                snapshot[entry.path().string()] = fs::last_write_time(entry);
            }
        }
        return snapshot;
    }

    void watch_loop() {
        while (running_) {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait_for(lock, interval_, [this] { return !running_; });
            
            if (!running_) break;

            auto new_snapshot = take_snapshot();
            detect_changes(new_snapshot);
            snapshot_ = std::move(new_snapshot);
        }
    }

    void detect_changes(const FileSnapshot& new_snapshot) {
        for (const auto& [path, time] : new_snapshot) {
            auto old_it = snapshot_.find(path);
            if (old_it == snapshot_.end()) {
                std::cout << "File created: " << path << std::endl;
            } else if (old_it->second != time) {
                std::cout << "File modified: " << path << std::endl;
            }
        }

        for (const auto& [path, time] : snapshot_) {
            if (new_snapshot.find(path) == new_snapshot.end()) {
                std::cout << "File deleted: " << path << std::endl;
            }
        }
    }

    fs::path watch_path_;
    std::chrono::milliseconds interval_;
    std::atomic<bool> running_;
    std::thread watcher_thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    FileSnapshot snapshot_;
};

int main() {
    try {
        FileSystemWatcher watcher(".", std::chrono::seconds(2));
        watcher.start();
        
        std::this_thread::sleep_for(std::chrono::seconds(10));
        
        watcher.stop();
        std::cout << "File system watcher stopped." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}