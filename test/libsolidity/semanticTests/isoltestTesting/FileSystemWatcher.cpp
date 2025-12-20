
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    SimpleFileWatcher(const fs::path& watch_path, std::chrono::milliseconds interval)
        : watch_path_(watch_path), interval_(interval), running_(false) {
        if (!fs::exists(watch_path)) {
            throw std::runtime_error("Watch path does not exist");
        }
        snapshot_ = take_snapshot();
    }

    void start() {
        running_ = true;
        watcher_thread_ = std::thread(&SimpleFileWatcher::watch_loop, this);
    }

    void stop() {
        running_ = false;
        if (watcher_thread_.joinable()) {
            watcher_thread_.join();
        }
    }

    ~SimpleFileWatcher() {
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
            std::this_thread::sleep_for(interval_);
            auto current_snapshot = take_snapshot();

            for (const auto& [path, time] : current_snapshot) {
                auto old_it = snapshot_.find(path);
                if (old_it == snapshot_.end()) {
                    std::cout << "[NEW] " << path << std::endl;
                } else if (old_it->second != time) {
                    std::cout << "[MODIFIED] " << path << std::endl;
                }
            }

            for (const auto& [path, time] : snapshot_) {
                if (current_snapshot.find(path) == current_snapshot.end()) {
                    std::cout << "[DELETED] " << path << std::endl;
                }
            }

            snapshot_ = std::move(current_snapshot);
        }
    }

    fs::path watch_path_;
    std::chrono::milliseconds interval_;
    FileSnapshot snapshot_;
    std::thread watcher_thread_;
    std::atomic<bool> running_;
};

int main() {
    try {
        SimpleFileWatcher watcher(".", std::chrono::seconds(2));
        watcher.start();

        std::cout << "Watching current directory. Press Enter to stop..." << std::endl;
        std::cin.get();

        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}