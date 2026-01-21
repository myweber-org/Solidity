
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path watch_path;
    std::unordered_set<std::string> known_files;
    bool running;

    void scan_directory() {
        std::unordered_set<std::string> current_files;
        
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.status())) {
                std::string filename = entry.path().filename().string();
                current_files.insert(filename);
                
                if (known_files.find(filename) == known_files.end()) {
                    std::cout << "File added: " << filename << std::endl;
                }
            }
        }

        for (const auto& old_file : known_files) {
            if (current_files.find(old_file) == current_files.end()) {
                std::cout << "File removed: " << old_file << std::endl;
            }
        }

        known_files = std::move(current_files);
    }

public:
    FileSystemWatcher(const std::string& path) : watch_path(path), running(false) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Invalid directory path");
        }
    }

    void start() {
        running = true;
        std::cout << "Starting file system watcher for: " << watch_path.string() << std::endl;
        
        scan_directory();

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            scan_directory();
        }
    }

    void stop() {
        running = false;
        std::cout << "File system watcher stopped" << std::endl;
    }

    ~FileSystemWatcher() {
        if (running) {
            stop();
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
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
            watch_paths_[path] = callback;
            for (const auto& entry : fs::recursive_directory_iterator(path)) {
                file_timestamps_[entry.path()] = fs::last_write_time(entry);
            }
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
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::lock_guard<std::mutex> lock(mutex_);

            for (const auto& [path, callback] : watch_paths_) {
                if (!fs::exists(path)) continue;

                std::unordered_map<fs::path, fs::file_time_type> current_timestamps;

                for (const auto& entry : fs::recursive_directory_iterator(path)) {
                    current_timestamps[entry.path()] = fs::last_write_time(entry);
                }

                for (const auto& [file_path, new_timestamp] : current_timestamps) {
                    auto it = file_timestamps_.find(file_path);
                    if (it == file_timestamps_.end()) {
                        callback(file_path, "CREATED");
                    } else if (it->second != new_timestamp) {
                        callback(file_path, "MODIFIED");
                    }
                }

                for (const auto& [file_path, old_timestamp] : file_timestamps_) {
                    if (current_timestamps.find(file_path) == current_timestamps.end()) {
                        callback(file_path, "DELETED");
                    }
                }

                file_timestamps_.swap(current_timestamps);
            }
        }
    }

    std::unordered_map<fs::path, Callback> watch_paths_;
    std::unordered_map<fs::path, fs::file_time_type> file_timestamps_;
    std::atomic<bool> running_;
    std::thread monitor_thread_;
    std::mutex mutex_;
};

void exampleCallback(const fs::path& path, const std::string& action) {
    std::cout << "File: " << path.string() << " Action: " << action << std::endl;
}

int main() {
    FileSystemWatcher watcher;
    watcher.addWatchPath("./test_directory", exampleCallback);
    watcher.start();

    std::this_thread::sleep_for(std::chrono::seconds(30));
    watcher.stop();

    return 0;
}