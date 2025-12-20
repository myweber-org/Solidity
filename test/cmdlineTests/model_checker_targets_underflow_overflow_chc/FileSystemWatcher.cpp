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
            watch_paths_[path] = callback;
            file_timestamps_[path] = getLastWriteTime(path);
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
    std::unordered_map<fs::path, Callback> watch_paths_;
    std::unordered_map<fs::path, fs::file_time_type> file_timestamps_;
    std::atomic<bool> running_;
    std::thread monitor_thread_;
    std::mutex mutex_;

    fs::file_time_type getLastWriteTime(const fs::path& path) {
        if (fs::exists(path)) {
            return fs::last_write_time(path);
        }
        return fs::file_time_type::min();
    }

    void monitor() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto& [path, callback] : watch_paths_) {
                try {
                    auto current_time = getLastWriteTime(path);
                    auto last_time = file_timestamps_[path];

                    if (current_time != last_time) {
                        file_timestamps_[path] = current_time;
                        callback(path, "modified");

                        if (!fs::exists(path)) {
                            callback(path, "deleted");
                        }
                    }

                    for (const auto& entry : fs::recursive_directory_iterator(path)) {
                        auto entry_path = entry.path();
                        auto it = file_timestamps_.find(entry_path);

                        if (it == file_timestamps_.end()) {
                            file_timestamps_[entry_path] = getLastWriteTime(entry_path);
                            callback(entry_path, "created");
                        } else {
                            auto current_entry_time = getLastWriteTime(entry_path);
                            if (current_entry_time != it->second) {
                                it->second = current_entry_time;
                                callback(entry_path, "modified");
                            }
                        }
                    }
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Filesystem error: " << e.what() << std::endl;
                }
            }
        }
    }
};

int main() {
    FileSystemWatcher watcher;

    watcher.addWatchPath("./logs", [](const fs::path& path, const std::string& action) {
        std::cout << "File " << path << " was " << action << std::endl;
    });

    watcher.addWatchPath("./config", [](const fs::path& path, const std::string& action) {
        std::cout << "Config file " << path << " was " << action << std::endl;
    });

    std::cout << "Starting file system watcher. Press Enter to stop." << std::endl;
    watcher.start();

    std::cin.get();
    watcher.stop();
    std::cout << "File system watcher stopped." << std::endl;

    return 0;
}