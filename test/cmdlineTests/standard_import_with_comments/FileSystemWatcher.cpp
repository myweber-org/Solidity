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
        if (fs::exists(path) && fs::is_directory(path)) {
            watch_paths_[path] = {callback, fs::last_write_time(path)};
        }
    }

    void start() {
        running_ = true;
        monitor_thread_ = std::thread(&FileSystemWatcher::monitor, this);
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

    void monitor() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            std::lock_guard<std::mutex> lock(mutex_);
            for (auto& [path, info] : watch_paths_) {
                try {
                    if (!fs::exists(path)) {
                        info.callback(path, "deleted");
                        continue;
                    }

                    auto current_time = fs::last_write_time(path);
                    if (current_time != info.last_write_time) {
                        info.last_write_time = current_time;
                        std::string event_type = fs::is_directory(path) ? "directory_modified" : "file_modified";
                        info.callback(path, event_type);
                    }
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Filesystem error: " << e.what() << '\n';
                }
            }
        }
    }
};

void exampleCallback(const fs::path& path, const std::string& event) {
    std::cout << "Detected change: " << path << " - Event: " << event << std::endl;
}

int main() {
    FileSystemWatcher watcher;

    watcher.addWatchPath("./test_directory", exampleCallback);
    watcher.addWatchPath("./config.json", exampleCallback);

    std::cout << "Starting file system watcher. Monitoring for changes..." << std::endl;
    watcher.start();

    std::this_thread::sleep_for(std::chrono::seconds(30));

    watcher.stop();
    std::cout << "File system watcher stopped." << std::endl;

    return 0;
}