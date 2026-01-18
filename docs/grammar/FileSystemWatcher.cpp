
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const fs::path& path_to_watch) : path_to_watch_(path_to_watch) {
        for (auto& entry : fs::directory_iterator(path_to_watch_)) {
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
    fs::path path_to_watch_;
    std::unordered_map<fs::path, fs::file_time_type> paths_;
    bool running_ = true;

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

        for (auto& entry : fs::directory_iterator(path_to_watch_)) {
            auto current_last_write_time = fs::last_write_time(entry);

            if (!paths_.contains(entry.path())) {
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
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        std::cout << "Monitoring current directory for changes. Press Ctrl+C to stop." << std::endl;
        watcher.startMonitoring(2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}