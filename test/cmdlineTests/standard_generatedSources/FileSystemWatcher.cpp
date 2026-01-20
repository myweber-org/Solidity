#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const fs::path& path_to_watch) : path_to_watch_(path_to_watch) {
        for (const auto& entry : fs::recursive_directory_iterator(path_to_watch_)) {
            if (fs::is_regular_file(entry.path())) {
                known_files_.insert(fs::canonical(entry.path()));
            }
        }
    }

    void startMonitoring(int interval_seconds = 1) {
        std::cout << "Starting to monitor: " << path_to_watch_ << std::endl;
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
    std::unordered_set<std::string> known_files_;
    bool running_ = true;

    void checkForChanges() {
        std::unordered_set<std::string> current_files;

        for (const auto& entry : fs::recursive_directory_iterator(path_to_watch_)) {
            if (fs::is_regular_file(entry.path())) {
                std::string canonical_path = fs::canonical(entry.path());
                current_files.insert(canonical_path);

                if (known_files_.find(canonical_path) == known_files_.end()) {
                    std::cout << "File created: " << canonical_path << std::endl;
                    known_files_.insert(canonical_path);
                }
            }
        }

        std::unordered_set<std::string> files_to_remove;
        for (const auto& old_file : known_files_) {
            if (current_files.find(old_file) == current_files.end()) {
                std::cout << "File deleted: " << old_file << std::endl;
                files_to_remove.insert(old_file);
            }
        }

        for (const auto& file_to_remove : files_to_remove) {
            known_files_.erase(file_to_remove);
        }
    }
};

int main() {
    fs::path directory_to_watch = "./watch_directory";
    
    if (!fs::exists(directory_to_watch)) {
        if (fs::create_directory(directory_to_watch)) {
            std::cout << "Created directory: " << directory_to_watch << std::endl;
        } else {
            std::cerr << "Failed to create directory." << std::endl;
            return 1;
        }
    }

    FileSystemWatcher watcher(directory_to_watch);
    
    std::thread monitor_thread([&watcher]() {
        watcher.startMonitoring(2);
    });

    std::cout << "File system watcher is running. Press Enter to stop..." << std::endl;
    std::cin.get();
    
    watcher.stopMonitoring();
    monitor_thread.join();
    
    std::cout << "Monitoring stopped." << std::endl;
    return 0;
}