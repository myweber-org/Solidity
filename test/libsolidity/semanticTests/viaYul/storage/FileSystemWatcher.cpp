
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>
#include <functional>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using FileTime = fs::file_time_type;
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher(const fs::path& watch_path, Callback callback)
        : watch_path_(watch_path), callback_(callback), running_(false) {
        if (!fs::exists(watch_path_) || !fs::is_directory(watch_path_)) {
            throw std::runtime_error("Watch path does not exist or is not a directory");
        }
        snapshot_files();
    }

    void start() {
        running_ = true;
        monitor_thread_ = std::thread(&FileSystemWatcher::monitor_loop, this);
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
    void snapshot_files() {
        file_snapshot_.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (entry.is_regular_file()) {
                file_snapshot_[entry.path()] = entry.last_write_time();
            }
        }
    }

    void monitor_loop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            check_for_changes();
        }
    }

    void check_for_changes() {
        std::unordered_map<fs::path, FileTime> current_snapshot;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (entry.is_regular_file()) {
                current_snapshot[entry.path()] = entry.last_write_time();
            }
        }

        for (const auto& [path, current_time] : current_snapshot) {
            auto it = file_snapshot_.find(path);
            if (it == file_snapshot_.end()) {
                if (callback_) callback_(path, "created");
            } else if (it->second != current_time) {
                if (callback_) callback_(path, "modified");
            }
        }

        for (const auto& [path, old_time] : file_snapshot_) {
            if (current_snapshot.find(path) == current_snapshot.end()) {
                if (callback_) callback_(path, "deleted");
            }
        }

        file_snapshot_.swap(current_snapshot);
    }

    fs::path watch_path_;
    Callback callback_;
    std::unordered_map<fs::path, FileTime> file_snapshot_;
    std::thread monitor_thread_;
    std::atomic<bool> running_;
};

int main() {
    try {
        FileSystemWatcher watcher(fs::current_path(), [](const fs::path& path, const std::string& action) {
            std::cout << "File " << path.filename() << " was " << action << std::endl;
        });

        std::cout << "Watching directory: " << fs::current_path() << std::endl;
        std::cout << "Press Enter to stop watching..." << std::endl;

        watcher.start();
        std::cin.get();
        watcher.stop();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}