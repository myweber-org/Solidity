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
    void monitor() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::lock_guard<std::mutex> lock(mutex_);

            for (auto& [path, data] : watch_paths_) {
                if (!fs::exists(path)) continue;

                auto current_time = fs::last_write_time(path);
                if (data.last_write_time != current_time) {
                    data.callback(path, "modified");
                    data.last_write_time = current_time;
                }

                for (const auto& entry : fs::recursive_directory_iterator(path)) {
                    const auto& entry_path = entry.path();
                    if (!tracked_files_.count(entry_path)) {
                        if (fs::is_regular_file(entry_path)) {
                            tracked_files_[entry_path] = fs::last_write_time(entry_path);
                            data.callback(entry_path, "created");
                        }
                    } else {
                        auto old_time = tracked_files_[entry_path];
                        auto new_time = fs::last_write_time(entry_path);
                        if (old_time != new_time) {
                            tracked_files_[entry_path] = new_time;
                            data.callback(entry_path, "modified");
                        }
                    }
                }

                auto it = tracked_files_.begin();
                while (it != tracked_files_.end()) {
                    if (!fs::exists(it->first)) {
                        data.callback(it->first, "deleted");
                        it = tracked_files_.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
        }
    }

    struct WatchData {
        Callback callback;
        fs::file_time_type last_write_time;
    };

    std::unordered_map<fs::path, WatchData> watch_paths_;
    std::unordered_map<fs::path, fs::file_time_type> tracked_files_;
    std::atomic<bool> running_;
    std::thread monitor_thread_;
    std::mutex mutex_;
};

void exampleCallback(const fs::path& path, const std::string& action) {
    std::cout << "File " << path << " has been " << action << std::endl;
}

int main() {
    FileSystemWatcher watcher;
    watcher.addWatchPath("./test_directory", exampleCallback);
    watcher.start();

    std::this_thread::sleep_for(std::chrono::seconds(30));
    watcher.stop();

    return 0;
}