
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    using FileTimeMap = std::unordered_map<std::string, fs::file_time_type>;

    SimpleFileWatcher(const std::string& path) : watch_path_(path), running_(false) {
        if (!fs::exists(path)) {
            throw std::runtime_error("Path does not exist: " + path);
        }
    }

    void start() {
        running_ = true;
        snapshot_ = take_snapshot();
        std::cout << "Watching directory: " << watch_path_ << std::endl;

        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            check_for_changes();
        }
    }

    void stop() {
        running_ = false;
    }

private:
    FileTimeMap take_snapshot() {
        FileTimeMap snapshot;
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (entry.is_regular_file()) {
                snapshot[entry.path().string()] = fs::last_write_time(entry);
            }
        }
        return snapshot;
    }

    void check_for_changes() {
        auto current_state = take_snapshot();

        for (const auto& [path, mtime] : current_state) {
            auto it = snapshot_.find(path);
            if (it == snapshot_.end()) {
                std::cout << "[NEW] " << path << std::endl;
            } else if (it->second != mtime) {
                std::cout << "[MODIFIED] " << path << std::endl;
            }
        }

        for (const auto& [path, mtime] : snapshot_) {
            if (current_state.find(path) == current_state.end()) {
                std::cout << "[DELETED] " << path << std::endl;
            }
        }

        snapshot_ = std::move(current_state);
    }

    std::string watch_path_;
    FileTimeMap snapshot_;
    bool running_;
};

int main() {
    try {
        SimpleFileWatcher watcher(".");
        watcher.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}