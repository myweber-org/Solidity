
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

    void addWatchPath(const fs::path& path, Callback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (fs::exists(path)) {
            watch_paths_[path] = {callback, fs::last_write_time(path)};
        }
    }

    void removeWatchPath(const fs::path& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        watch_paths_.erase(path);
    }

    void start() {
        running_ = true;
        monitor_thread_ = std::thread(&FileSystemWatcher::monitorLoop, this);
    }

    void stop() {
        running_ = false;
        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

private:
    struct WatchInfo {
        Callback callback;
        fs::file_time_type last_write_time;
    };

    std::unordered_map<fs::path, WatchInfo> watch_paths_;
    std::atomic<bool> running_;
    std::thread monitor_thread_;
    std::mutex mutex_;

    void monitorLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            std::lock_guard<std::mutex> lock(mutex_);
            for (auto& [path, info] : watch_paths_) {
                if (!fs::exists(path)) {
                    info.callback(path, "deleted");
                    continue;
                }

                auto current_time = fs::last_write_time(path);
                if (current_time != info.last_write_time) {
                    info.last_write_time = current_time;
                    info.callback(path, "modified");
                }
            }
        }
    }
};

void exampleCallback(const fs::path& path, const std::string& action) {
    std::cout << "File " << path << " was " << action << " at "
              << std::chrono::system_clock::now().time_since_epoch().count()
              << " nanoseconds since epoch\n";
}

int main() {
    FileSystemWatcher watcher;

    watcher.addWatchPath("test_file.txt", exampleCallback);
    watcher.addWatchPath("config.json", exampleCallback);

    watcher.start();

    std::cout << "Watching files for changes. Press Enter to stop...\n";
    std::cin.get();

    watcher.stop();
    return 0;
}