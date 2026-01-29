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
        if (!fs::exists(path) || !fs::is_directory(path)) {
            throw std::runtime_error("Path does not exist or is not a directory");
        }
    }

    void start() {
        running_ = true;
        snapshot_ = take_snapshot();
        std::cout << "Watching directory: " << watch_path_ << std::endl;

        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
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
            if (fs::is_regular_file(entry.path())) {
                snapshot[entry.path().string()] = fs::last_write_time(entry);
            }
        }
        return snapshot;
    }

    void check_for_changes() {
        auto current_snapshot = take_snapshot();

        for (const auto& [path, time] : current_snapshot) {
            auto it = snapshot_.find(path);
            if (it == snapshot_.end()) {
                std::cout << "[ADDED] " << path << std::endl;
            } else if (it->second != time) {
                std::cout << "[MODIFIED] " << path << std::endl;
            }
        }

        for (const auto& [path, time] : snapshot_) {
            if (current_snapshot.find(path) == current_snapshot.end()) {
                std::cout << "[REMOVED] " << path << std::endl;
            }
        }

        snapshot_ = std::move(current_snapshot);
    }

    std::string watch_path_;
    FileTimeMap snapshot_;
    bool running_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        SimpleFileWatcher watcher(argv[1]);
        watcher.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}