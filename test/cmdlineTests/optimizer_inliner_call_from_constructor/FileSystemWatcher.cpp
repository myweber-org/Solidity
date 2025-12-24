#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <iostream>
#include <functional>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher(const fs::path& watch_path, Callback callback)
        : watch_path_(watch_path), callback_(callback), running_(false) {
        if (!fs::exists(watch_path_) || !fs::is_directory(watch_path_)) {
            throw std::runtime_error("Watch path does not exist or is not a directory");
        }
        snapshot_ = takeSnapshot();
    }

    ~FileSystemWatcher() {
        stop();
    }

    void start(int interval_ms = 1000) {
        running_ = true;
        monitor_thread_ = std::thread([this, interval_ms]() {
            while (running_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
                checkForChanges();
            }
        });
    }

    void stop() {
        running_ = false;
        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
    }

private:
    using FileSnapshot = std::unordered_map<std::string, fs::file_time_type>;

    FileSnapshot takeSnapshot() {
        FileSnapshot snapshot;
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (entry.is_regular_file()) {
                std::error_code ec;
                auto ftime = entry.last_write_time(ec);
                if (!ec) {
                    snapshot[entry.path().string()] = ftime;
                }
            }
        }
        return snapshot;
    }

    void checkForChanges() {
        auto current_snapshot = takeSnapshot();

        // Check for modified or created files
        for (const auto& [path, current_time] : current_snapshot) {
            auto it = snapshot_.find(path);
            if (it == snapshot_.end()) {
                // New file
                if (callback_) callback_(path, "created");
            } else if (it->second != current_time) {
                // Modified file
                if (callback_) callback_(path, "modified");
            }
        }

        // Check for deleted files
        for (const auto& [path, old_time] : snapshot_) {
            if (current_snapshot.find(path) == current_snapshot.end()) {
                // Deleted file
                if (callback_) callback_(path, "deleted");
            }
        }

        snapshot_ = std::move(current_snapshot);
    }

    fs::path watch_path_;
    Callback callback_;
    FileSnapshot snapshot_;
    std::thread monitor_thread_;
    std::atomic<bool> running_;
};

// Example usage (commented out as per instructions)
/*
int main() {
    FileSystemWatcher watcher(".", [](const fs::path& path, const std::string& action) {
        std::cout << "File " << path << " was " << action << std::endl;
    });

    watcher.start();

    std::this_thread::sleep_for(std::chrono::seconds(30));

    watcher.stop();
    return 0;
}
*/