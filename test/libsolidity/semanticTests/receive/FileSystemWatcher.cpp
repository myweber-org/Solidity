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
    FileSystemWatcher(const fs::path& watch_path, std::chrono::milliseconds interval)
        : watch_path_(watch_path), interval_(interval), running_(false) {
        if (!fs::exists(watch_path_) || !fs::is_directory(watch_path_)) {
            throw std::runtime_error("Invalid directory path");
        }
        snapshot_ = takeSnapshot();
    }

    void start() {
        running_ = true;
        monitor_thread_ = std::thread(&FileSystemWatcher::monitorLoop, this);
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            running_ = false;
        }
        cv_.notify_all();
        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
    }

    ~FileSystemWatcher() {
        if (running_) {
            stop();
        }
    }

private:
    using FileSnapshot = std::unordered_set<std::string>;

    FileSnapshot takeSnapshot() {
        FileSnapshot snapshot;
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (fs::is_regular_file(entry.path())) {
                snapshot.insert(fs::absolute(entry.path()).string());
            }
        }
        return snapshot;
    }

    void monitorLoop() {
        while (running_) {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait_for(lock, interval_, [this]() { return !running_; });
            
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
            std::lock_guard<std::mutex> lock(print_mutex_);
            std::cout << "[" << std::chrono::system_clock::now().time_since_epoch().count()
                      << "] File system changes detected:" << std::endl;
            
            for (const auto& file : added_files) {
                std::cout << "  [+] " << file << std::endl;
            }
            
            for (const auto& file : removed_files) {
                std::cout << "  [-] " << file << std::endl;
            }
        }
    }

    fs::path watch_path_;
    std::chrono::milliseconds interval_;
    std::atomic<bool> running_;
    FileSnapshot snapshot_;
    std::thread monitor_thread_;
    std::condition_variable cv_;
    std::mutex mutex_;
    std::mutex print_mutex_;
};

int main() {
    try {
        fs::path current_path = fs::current_path();
        FileSystemWatcher watcher(current_path, std::chrono::seconds(2));
        
        std::cout << "Starting file system watcher for: " << current_path << std::endl;
        std::cout << "Monitoring interval: 2 seconds" << std::endl;
        std::cout << "Press Enter to stop..." << std::endl;
        
        watcher.start();
        std::cin.get();
        watcher.stop();
        
        std::cout << "File system watcher stopped." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>
#include <functional>
#include <iostream>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher(const fs::path& watch_path, Callback callback)
        : watch_path_(watch_path), callback_(callback), running_(false) {
        if (!fs::exists(watch_path_)) {
            throw std::runtime_error("Watch path does not exist");
        }
        scan_files();
    }

    ~FileSystemWatcher() {
        stop();
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

private:
    fs::path watch_path_;
    Callback callback_;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps_;
    bool running_;
    std::thread monitor_thread_;

    void scan_files() {
        file_timestamps_.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (entry.is_regular_file()) {
                auto path = entry.path();
                auto timestamp = fs::last_write_time(path);
                file_timestamps_[path.string()] = timestamp;
            }
        }
    }

    void monitor() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
                if (!entry.is_regular_file()) continue;

                auto path = entry.path();
                auto current_time = fs::last_write_time(path);
                auto path_str = path.string();

                auto it = file_timestamps_.find(path_str);
                if (it == file_timestamps_.end()) {
                    file_timestamps_[path_str] = current_time;
                    if (callback_) callback_(path, "created");
                } else if (it->second != current_time) {
                    it->second = current_time;
                    if (callback_) callback_(path, "modified");
                }
            }

            std::vector<std::string> to_remove;
            for (const auto& [path_str, timestamp] : file_timestamps_) {
                if (!fs::exists(path_str)) {
                    to_remove.push_back(path_str);
                }
            }

            for (const auto& path_str : to_remove) {
                file_timestamps_.erase(path_str);
                if (callback_) callback_(fs::path(path_str), "deleted");
            }
        }
    }
};