
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& path) : watch_path_(path) {
        if (!fs::exists(watch_path_) || !fs::is_directory(watch_path_)) {
            throw std::runtime_error("Path does not exist or is not a directory");
        }
        populate_snapshot();
    }

    void start_watching(int interval_seconds = 1) {
        std::cout << "Watching directory: " << watch_path_ << std::endl;
        
        while (!stop_requested_) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

    void stop() {
        stop_requested_ = true;
    }

private:
    void populate_snapshot() {
        file_snapshot_.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (fs::is_regular_file(entry.status())) {
                file_snapshot_[entry.path()] = fs::last_write_time(entry);
            }
        }
    }

    void check_for_changes() {
        std::unordered_map<fs::path, fs::file_time_type> current_state;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (fs::is_regular_file(entry.status())) {
                current_state[entry.path()] = fs::last_write_time(entry);
            }
        }

        for (const auto& [path, timestamp] : current_state) {
            auto it = file_snapshot_.find(path);
            if (it == file_snapshot_.end()) {
                std::cout << "[NEW] " << path.filename() << std::endl;
            } else if (it->second != timestamp) {
                std::cout << "[MODIFIED] " << path.filename() << std::endl;
            }
        }

        for (const auto& [path, timestamp] : file_snapshot_) {
            if (current_state.find(path) == current_state.end()) {
                std::cout << "[DELETED] " << path.filename() << std::endl;
            }
        }

        file_snapshot_.swap(current_state);
    }

    fs::path watch_path_;
    std::unordered_map<fs::path, fs::file_time_type> file_snapshot_;
    bool stop_requested_{false};
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start_watching(2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}