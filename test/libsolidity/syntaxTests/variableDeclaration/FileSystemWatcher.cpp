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
            auto it = paths_.begin();
            while (it != paths_.end()) {
                if (!fs::exists(it->first)) {
                    std::cout << "File removed: " << it->first << std::endl;
                    it = paths_.erase(it);
                } else {
                    ++it;
                }
            }

            for (const auto& entry : fs::directory_iterator(path_to_watch_)) {
                auto current_file_last_write_time = fs::last_write_time(entry);
                if (paths_.find(entry.path()) == paths_.end()) {
                    paths_[entry.path()] = current_file_last_write_time;
                    std::cout << "File added: " << entry.path() << std::endl;
                } else {
                    if (paths_[entry.path()] != current_file_last_write_time) {
                        paths_[entry.path()] = current_file_last_write_time;
                        std::cout << "File modified: " << entry.path() << std::endl;
                    }
                }
            }
        }
    }

    void stopMonitoring() {
        running_ = false;
    }

private:
    fs::path path_to_watch_;
    std::unordered_map<fs::path, fs::file_time_type> paths_;
    bool running_ = true;
};

int main() {
    fs::path path_to_watch = "./watch_dir";
    if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
        std::cerr << "Directory does not exist or is not a directory: " << path_to_watch << std::endl;
        return 1;
    }

    FileSystemWatcher watcher(path_to_watch);
    std::cout << "Starting to monitor directory: " << path_to_watch << std::endl;
    watcher.startMonitoring(2);
    return 0;
}