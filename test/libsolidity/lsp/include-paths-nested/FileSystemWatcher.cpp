
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

    SimpleFileWatcher(const std::string& path, int interval_ms = 1000)
        : watch_path_(path), interval_ms_(interval_ms), running_(false) {}

    void start() {
        running_ = true;
        snapshot_ = take_snapshot();
        std::cout << "Watching directory: " << watch_path_ << std::endl;

        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms_));
            check_for_changes();
        }
    }

    void stop() {
        running_ = false;
    }

private:
    FileTimeMap take_snapshot() {
        FileTimeMap snapshot;
        try {
            for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
                if (entry.is_regular_file()) {
                    snapshot[entry.path().string()] = fs::last_write_time(entry);
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
        return snapshot;
    }

    void check_for_changes() {
        auto current_state = take_snapshot();

        for (const auto& [path, time] : current_state) {
            auto old_it = snapshot_.find(path);
            if (old_it == snapshot_.end()) {
                std::cout << "[NEW] " << path << std::endl;
            } else if (old_it->second != time) {
                std::cout << "[MODIFIED] " << path << std::endl;
            }
        }

        for (const auto& [path, time] : snapshot_) {
            if (current_state.find(path) == current_state.end()) {
                std::cout << "[DELETED] " << path << std::endl;
            }
        }

        snapshot_ = std::move(current_state);
    }

    std::string watch_path_;
    int interval_ms_;
    bool running_;
    FileTimeMap snapshot_;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    std::string watch_dir = argv[1];
    if (!fs::exists(watch_dir) || !fs::is_directory(watch_dir)) {
        std::cerr << "Error: " << watch_dir << " is not a valid directory." << std::endl;
        return 1;
    }

    SimpleFileWatcher watcher(watch_dir, 2000);

    std::thread watch_thread([&watcher]() {
        watcher.start();
    });

    std::cout << "File watcher started. Press Enter to stop..." << std::endl;
    std::cin.get();

    watcher.stop();
    watch_thread.join();

    std::cout << "File watcher stopped." << std::endl;
    return 0;
}