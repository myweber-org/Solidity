
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& path_to_watch) : path_to_watch_(path_to_watch) {
        if (!fs::exists(path_to_watch_) || !fs::is_directory(path_to_watch_)) {
            throw std::runtime_error("Provided path is not a valid directory.");
        }
        populate_file_set();
    }

    void start_monitoring(int interval_seconds = 1) {
        std::cout << "Starting to monitor: " << path_to_watch_ << std::endl;
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

    void stop_monitoring() {
        running_ = false;
    }

private:
    fs::path path_to_watch_;
    std::unordered_set<std::string> current_files_;
    bool running_{true};

    void populate_file_set() {
        current_files_.clear();
        for (const auto& entry : fs::directory_iterator(path_to_watch_)) {
            current_files_.insert(entry.path().filename().string());
        }
    }

    void check_for_changes() {
        auto new_files = current_files_;
        bool changed = false;

        for (const auto& entry : fs::directory_iterator(path_to_watch_)) {
            std::string filename = entry.path().filename().string();
            if (current_files_.find(filename) == current_files_.end()) {
                std::cout << "File added: " << filename << std::endl;
                changed = true;
            } else {
                new_files.erase(filename);
            }
        }

        for (const auto& missing : new_files) {
            std::cout << "File removed: " << missing << std::endl;
            changed = true;
        }

        if (changed) {
            current_files_.clear();
            for (const auto& entry : fs::directory_iterator(path_to_watch_)) {
                current_files_.insert(entry.path().filename().string());
            }
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start_monitoring(2);
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}