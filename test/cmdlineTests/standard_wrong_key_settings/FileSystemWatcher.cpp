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
            std::cerr << "Path does not exist or is not a directory: " << path << std::endl;
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
                    if (entry.is_regular_file()) {
                        auto path = entry.path();
                        file_snapshot_[path.string()] = fs::last_write_time(path);
                    }
                }
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Filesystem error: " << e.what() << std::endl;
            }
        }
    }

    void watchLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            std::lock_guard<std::mutex> lock(mutex_);
            std::unordered_map<std::string, fs::file_time_type> current_state;

            for (const auto& dir : watch_paths_) {
                try {
                    for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                        if (entry.is_regular_file()) {
                            auto path = entry.path();
                            current_state[path.string()] = fs::last_write_time(path);
                        }
                    }
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Filesystem error: " << e.what() << std::endl;
                }
            }

            detectChanges(current_state);
            file_snapshot_.swap(current_state);
        }
    }

    void detectChanges(const std::unordered_map<std::string, fs::file_time_type>& current) {
        for (const auto& [path, mtime] : current) {
            auto it = file_snapshot_.find(path);
            if (it == file_snapshot_.end()) {
                notify(path, "created");
            } else if (it->second != mtime) {
                notify(path, "modified");
            }
        }

        for (const auto& [path, _] : file_snapshot_) {
            if (current.find(path) == current.end()) {
                notify(path, "deleted");
            }
        }
    }

    void notify(const std::string& path, const std::string& action) {
        if (callback_) {
            callback_(path, action);
        } else {
            std::cout << "File " << action << ": " << path << std::endl;
        }
    }
};

int main() {
    FileSystemWatcher watcher;
    watcher.addWatchPath(".");
    
    watcher.setCallback([](const fs::path& path, const std::string& action) {
        std::cout << "[" << action << "] " << path.filename() << std::endl;
    });

    watcher.start();
    std::cout << "File system watcher started. Monitoring for changes..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(30));
    watcher.stop();
    std::cout << "File system watcher stopped." << std::endl;

    return 0;
}