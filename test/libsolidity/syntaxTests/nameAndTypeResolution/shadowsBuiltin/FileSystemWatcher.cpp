#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <mutex>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher() : running_(false) {}

    void addWatchPath(const fs::path& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (fs::exists(path) && fs::is_directory(path)) {
            watch_paths_.push_back(fs::canonical(path));
            std::cout << "Watching directory: " << watch_paths_.back() << std::endl;
        } else {
            std::cerr << "Invalid or non-existent directory: " << path << std::endl;
        }
    }

    void setCallback(Callback cb) {
        callback_ = std::move(cb);
    }

    void start() {
        if (running_) return;
        running_ = true;
        snapshotFiles();
        worker_thread_ = std::thread(&FileSystemWatcher::watchLoop, this);
    }

    void stop() {
        running_ = false;
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

private:
    std::vector<fs::path> watch_paths_;
    std::unordered_map<std::string, fs::file_time_type> file_snapshot_;
    Callback callback_;
    std::atomic<bool> running_;
    std::thread worker_thread_;
    std::mutex mutex_;

    void snapshotFiles() {
        file_snapshot_.clear();
        for (const auto& dir : watch_paths_) {
            try {
                for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                    if (fs::is_regular_file(entry.status())) {
                        auto ftime = fs::last_write_time(entry.path());
                        file_snapshot_[entry.path().string()] = ftime;
                    }
                }
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Filesystem error: " << e.what() << std::endl;
            }
        }
    }

    void watchLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::lock_guard<std::mutex> lock(mutex_);
            checkForChanges();
        }
    }

    void checkForChanges() {
        std::unordered_map<std::string, fs::file_time_type> current_state;

        for (const auto& dir : watch_paths_) {
            try {
                for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                    if (fs::is_regular_file(entry.status())) {
                        auto path_str = entry.path().string();
                        auto ftime = fs::last_write_time(entry.path());
                        current_state[path_str] = ftime;

                        auto it = file_snapshot_.find(path_str);
                        if (it == file_snapshot_.end()) {
                            notifyChange(entry.path(), "created");
                        } else if (it->second != ftime) {
                            notifyChange(entry.path(), "modified");
                        }
                    }
                }
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Filesystem error during watch: " << e.what() << std::endl;
            }
        }

        for (const auto& [path, time] : file_snapshot_) {
            if (current_state.find(path) == current_state.end()) {
                notifyChange(fs::path(path), "deleted");
            }
        }

        file_snapshot_.swap(current_state);
    }

    void notifyChange(const fs::path& path, const std::string& action) {
        if (callback_) {
            callback_(path, action);
        }
        std::cout << "File " << action << ": " << path << std::endl;
    }
};

int main() {
    FileSystemWatcher watcher;
    watcher.addWatchPath(".");
    watcher.setCallback([](const fs::path& path, const std::string& action) {
        std::cout << "[Callback] " << action << " -> " << path.filename() << std::endl;
    });

    std::cout << "Starting file system watcher. Press Enter to stop..." << std::endl;
    watcher.start();

    std::cin.get();
    watcher.stop();
    std::cout << "Watcher stopped." << std::endl;

    return 0;
}