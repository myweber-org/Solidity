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

    FileSystemWatcher(const fs::path& watch_path, Callback callback)
        : watch_path_(watch_path), callback_(callback), running_(false) {
        if (!fs::exists(watch_path_) || !fs::is_directory(watch_path_)) {
            throw std::runtime_error("Watch path does not exist or is not a directory.");
        }
        scanExistingFiles();
    }

    ~FileSystemWatcher() {
        stop();
    }

    void start() {
        if (running_) return;
        running_ = true;
        watcher_thread_ = std::thread(&FileSystemWatcher::watchLoop, this);
    }

    void stop() {
        running_ = false;
        if (watcher_thread_.joinable()) {
            watcher_thread_.join();
        }
    }

private:
    fs::path watch_path_;
    Callback callback_;
    std::atomic<bool> running_;
    std::thread watcher_thread_;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps_;
    std::mutex mutex_;

    void scanExistingFiles() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (fs::is_regular_file(entry.status())) {
                file_timestamps_[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void watchLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            std::lock_guard<std::mutex> lock(mutex_);
            std::unordered_map<std::string, fs::file_time_type> current_timestamps;

            for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
                if (!fs::is_regular_file(entry.status())) continue;

                auto path_str = entry.path().string();
                auto current_time = fs::last_write_time(entry);
                current_timestamps[path_str] = current_time;

                auto it = file_timestamps_.find(path_str);
                if (it == file_timestamps_.end()) {
                    callback_(entry.path(), "created");
                } else if (it->second != current_time) {
                    callback_(entry.path(), "modified");
                }
            }

            for (const auto& [path_str, _] : file_timestamps_) {
                if (current_timestamps.find(path_str) == current_timestamps.end()) {
                    callback_(fs::path(path_str), "deleted");
                }
            }

            file_timestamps_.swap(current_timestamps);
        }
    }
};

void exampleCallback(const fs::path& path, const std::string& action) {
    std::cout << "File: " << path << " Action: " << action << std::endl;
}

int main() {
    try {
        FileSystemWatcher watcher(fs::current_path(), exampleCallback);
        watcher.start();

        std::cout << "Watching directory: " << fs::current_path() << std::endl;
        std::cout << "Press Enter to stop..." << std::endl;
        std::cin.get();

        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}