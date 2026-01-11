#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>
#include <mutex>

namespace fs = std::filesystem;

class FileWatcher {
public:
    FileWatcher(const fs::path& path_to_watch, std::chrono::milliseconds interval)
        : path_to_watch_(path_to_watch), interval_(interval), running_(false) {}

    void start() {
        running_ = true;
        snapshot_ = take_snapshot();
        watcher_thread_ = std::thread(&FileWatcher::watch, this);
    }

    void stop() {
        running_ = false;
        if (watcher_thread_.joinable()) {
            watcher_thread_.join();
        }
    }

    ~FileWatcher() {
        stop();
    }

private:
    fs::path path_to_watch_;
    std::chrono::milliseconds interval_;
    std::unordered_set<std::string> snapshot_;
    std::thread watcher_thread_;
    std::mutex snapshot_mutex_;
    bool running_;

    std::unordered_set<std::string> take_snapshot() {
        std::unordered_set<std::string> files;
        if (fs::exists(path_to_watch_) && fs::is_directory(path_to_watch_)) {
            for (const auto& entry : fs::directory_iterator(path_to_watch_)) {
                files.insert(entry.path().filename().string());
            }
        }
        return files;
    }

    void watch() {
        while (running_) {
            std::this_thread::sleep_for(interval_);
            auto current_snapshot = take_snapshot();

            std::lock_guard<std::mutex> lock(snapshot_mutex_);
            std::unordered_set<std::string> new_files, deleted_files;

            for (const auto& file : current_snapshot) {
                if (snapshot_.find(file) == snapshot_.end()) {
                    new_files.insert(file);
                }
            }

            for (const auto& file : snapshot_) {
                if (current_snapshot.find(file) == current_snapshot.end()) {
                    deleted_files.insert(file);
                }
            }

            if (!new_files.empty() || !deleted_files.empty()) {
                std::cout << "Change detected in: " << path_to_watch_ << std::endl;
                for (const auto& file : new_files) {
                    std::cout << "  [+] " << file << std::endl;
                }
                for (const auto& file : deleted_files) {
                    std::cout << "  [-] " << file << std::endl;
                }
                snapshot_ = std::move(current_snapshot);
            }
        }
    }
};

int main() {
    fs::path watch_path = "./watch_dir";
    fs::create_directories(watch_path);

    FileWatcher watcher(watch_path, std::chrono::milliseconds(2000));
    watcher.start();

    std::cout << "Watching directory: " << fs::absolute(watch_path) << std::endl;
    std::cout << "Press Enter to stop..." << std::endl;
    std::cin.get();

    watcher.stop();
    fs::remove_all(watch_path);
    return 0;
}