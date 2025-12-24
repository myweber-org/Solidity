#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& watch_path)
        : watch_path_(watch_path), running_(false) {
        if (!fs::exists(watch_path_) || !fs::is_directory(watch_path_)) {
            throw std::runtime_error("Invalid directory path");
        }
        snapshot_ = takeSnapshot();
    }

    void start() {
        running_ = true;
        watcher_thread_ = std::thread(&FileSystemWatcher::watchLoop, this);
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
        stop();
    }

private:
    using FileSnapshot = std::unordered_set<std::string>;

    FileSnapshot takeSnapshot() {
        FileSnapshot snapshot;
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (fs::is_regular_file(entry.path())) {
                snapshot.insert(fs::relative(entry.path(), watch_path_).string());
            }
        }
        return snapshot;
    }

    void watchLoop() {
        while (running_) {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait_for(lock, std::chrono::seconds(2), [this] { return !running_; });

            if (!running_) break;

            auto new_snapshot = takeSnapshot();
            detectChanges(new_snapshot);
            snapshot_ = std::move(new_snapshot);
        }
    }

    void detectChanges(const FileSnapshot& new_snapshot) {
        std::vector<std::string> added_files;
        std::vector<std::string> removed_files;

        for (const auto& file : new_snapshot) {
            if (snapshot_.find(file) == snapshot_.end()) {
                added_files.push_back(file);
            }
        }

        for (const auto& file : snapshot_) {
            if (new_snapshot.find(file) == new_snapshot.end()) {
                removed_files.push_back(file);
            }
        }

        if (!added_files.empty() || !removed_files.empty()) {
            std::cout << "File system changes detected in: " << watch_path_ << std::endl;
            
            if (!added_files.empty()) {
                std::cout << "  Added files:" << std::endl;
                for (const auto& file : added_files) {
                    std::cout << "    - " << file << std::endl;
                }
            }

            if (!removed_files.empty()) {
                std::cout << "  Removed files:" << std::endl;
                for (const auto& file : removed_files) {
                    std::cout << "    - " << file << std::endl;
                }
            }
            std::cout << std::endl;
        }
    }

    fs::path watch_path_;
    FileSnapshot snapshot_;
    std::atomic<bool> running_;
    std::thread watcher_thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        std::cout << "Watching current directory for file changes..." << std::endl;
        std::cout << "Press Enter to stop watching..." << std::endl;
        
        watcher.start();
        std::cin.get();
        watcher.stop();
        
        std::cout << "File system watcher stopped." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}