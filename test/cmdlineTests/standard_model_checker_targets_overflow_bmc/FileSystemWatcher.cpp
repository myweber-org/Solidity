
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
            path_cache_.insert(entry.path());
        }
    }

    void startMonitoring(int interval_seconds = 1) {
        std::cout << "Monitoring directory: " << path_to_watch_ << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            checkForChanges();
        }
    }

private:
    fs::path path_to_watch_;
    std::unordered_set<std::string> path_cache_;

    void checkForChanges() {
        auto current_paths = std::unordered_set<std::string>();
        for (const auto& entry : fs::directory_iterator(path_to_watch_)) {
            current_paths.insert(entry.path().string());
        }

        // Check for new files
        for (const auto& path : current_paths) {
            if (path_cache_.find(path) == path_cache_.end()) {
                std::cout << "[+] New file detected: " << path << std::endl;
            }
        }

        // Check for deleted files
        for (const auto& path : path_cache_) {
            if (current_paths.find(path) == current_paths.end()) {
                std::cout << "[-] File deleted: " << path << std::endl;
            }
        }

        path_cache_ = current_paths;
    }
};

int main() {
    FileSystemWatcher watcher(".");
    watcher.startMonitoring(2);
    return 0;
}