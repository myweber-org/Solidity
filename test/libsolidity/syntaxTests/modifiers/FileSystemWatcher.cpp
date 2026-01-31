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
    using FileTimeMap = std::unordered_map<std::string, fs::file_time_type>;
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher(const fs::path& directory, Callback callback)
        : watch_directory(directory), change_callback(callback), running(false) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        populate_snapshot();
    }

    ~FileSystemWatcher() {
        stop();
    }

    void start(int interval_seconds = 1) {
        running = true;
        monitor_thread = std::thread(&FileSystemWatcher::monitor_loop, this, interval_seconds);
    }

    void stop() {
        running = false;
        if (monitor_thread.joinable()) {
            monitor_thread.join();
        }
    }

private:
    void populate_snapshot() {
        file_snapshot.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.status())) {
                try {
                    file_snapshot[entry.path().string()] = fs::last_write_time(entry);
                } catch (const fs::filesystem_error&) {
                    // Skip files that cannot be accessed
                }
            }
        }
    }

    void monitor_loop(int interval_seconds) {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

    void check_for_changes() {
        FileTimeMap current_state;
        std::vector<std::string> created_files, modified_files, deleted_files;

        // Build current state
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.status())) {
                try {
                    current_state[entry.path().string()] = fs::last_write_time(entry);
                } catch (const fs::filesystem_error&) {
                    continue;
                }
            }
        }

        // Detect created and modified files
        for (const auto& [path, time] : current_state) {
            auto it = file_snapshot.find(path);
            if (it == file_snapshot.end()) {
                created_files.push_back(path);
            } else if (it->second != time) {
                modified_files.push_back(path);
            }
        }

        // Detect deleted files
        for (const auto& [path, time] : file_snapshot) {
            if (current_state.find(path) == current_state.end()) {
                deleted_files.push_back(path);
            }
        }

        // Update snapshot
        file_snapshot = std::move(current_state);

        // Trigger callbacks
        for (const auto& path : created_files) {
            change_callback(path, "created");
        }
        for (const auto& path : modified_files) {
            change_callback(path, "modified");
        }
        for (const auto& path : deleted_files) {
            change_callback(path, "deleted");
        }
    }

    fs::path watch_directory;
    Callback change_callback;
    FileTimeMap file_snapshot;
    std::thread monitor_thread;
    std::atomic<bool> running;
};

// Example usage (commented out as per request)
/*
int main() {
    FileSystemWatcher watcher("./test_directory", [](const fs::path& path, const std::string& action) {
        std::cout << "File: " << path << " Action: " << action << std::endl;
    });

    watcher.start(2);

    std::this_thread::sleep_for(std::chrono::seconds(30));
    watcher.stop();

    return 0;
}
*/