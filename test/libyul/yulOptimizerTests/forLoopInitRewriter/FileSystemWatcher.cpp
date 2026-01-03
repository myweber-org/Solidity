
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

    explicit SimpleFileWatcher(const std::string& path) : watch_path_(path) {
        if (fs::exists(path) && fs::is_directory(path)) {
            snapshot_ = take_snapshot();
        } else {
            std::cerr << "Path does not exist or is not a directory: " << path << std::endl;
        }
    }

    void start_watching(int interval_seconds = 2) {
        if (snapshot_.empty()) {
            std::cerr << "Cannot start watching. Invalid path or empty directory." << std::endl;
            return;
        }

        std::cout << "Watching directory: " << watch_path_ << std::endl;
        std::cout << "Polling interval: " << interval_seconds << " seconds" << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    FileTimeMap take_snapshot() {
        FileTimeMap snapshot;
        try {
            for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
                if (fs::is_regular_file(entry.path())) {
                    snapshot[entry.path().string()] = fs::last_write_time(entry);
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
        return snapshot;
    }

    void check_for_changes() {
        auto current_snapshot = take_snapshot();

        for (const auto& [path, current_time] : current_snapshot) {
            auto old_it = snapshot_.find(path);
            if (old_it == snapshot_.end()) {
                std::cout << "[NEW] File created: " << path << std::endl;
            } else if (old_it->second != current_time) {
                std::cout << "[MODIFIED] File changed: " << path << std::endl;
            }
        }

        for (const auto& [path, old_time] : snapshot_) {
            if (current_snapshot.find(path) == current_snapshot.end()) {
                std::cout << "[DELETED] File removed: " << path << std::endl;
            }
        }

        snapshot_ = std::move(current_snapshot);
    }

    std::string watch_path_;
    FileTimeMap snapshot_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        SimpleFileWatcher watcher(argv[1]);
        watcher.start_watching();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}