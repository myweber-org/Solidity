#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const fs::path& path_to_watch) : path_to_watch_(path_to_watch) {
        for (const auto& entry : fs::directory_iterator(path_to_watch_)) {
            paths_[entry.path()] = fs::last_write_time(entry);
        }
    }

    void startMonitoring(int interval_seconds = 1) {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            checkForChanges();
        }
    }

    void stopMonitoring() {
        running_ = false;
    }

private:
    void checkForChanges() {
        auto it = paths_.begin();
        while (it != paths_.end()) {
            if (!fs::exists(it->first)) {
                std::cout << "File deleted: " << it->first << std::endl;
                it = paths_.erase(it);
            } else {
                ++it;
            }
        }

        for (const auto& entry : fs::directory_iterator(path_to_watch_)) {
            auto current_last_write_time = fs::last_write_time(entry);

            if (!paths_.count(entry.path())) {
                paths_[entry.path()] = current_last_write_time;
                std::cout << "File created: " << entry.path() << std::endl;
            } else {
                if (paths_[entry.path()] != current_last_write_time) {
                    paths_[entry.path()] = current_last_write_time;
                    std::cout << "File modified: " << entry.path() << std::endl;
                }
            }
        }
    }

    fs::path path_to_watch_;
    std::unordered_map<fs::path, fs::file_time_type> paths_;
    bool running_ = true;
};

int main() {
    fs::path path_to_watch = "./watch_directory";
    if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
        std::cerr << "Directory does not exist or is not a directory: " << path_to_watch << std::endl;
        return 1;
    }

    FileSystemWatcher watcher(path_to_watch);
    std::cout << "Starting to monitor directory: " << path_to_watch << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;

    watcher.startMonitoring(2);

    return 0;
}