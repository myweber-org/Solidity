
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
        }
    }

    void setEventCallback(Callback callback) {
        callback_ = std::move(callback);
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
    void monitorLoop() {
        std::unordered_map<fs::path, fs::file_time_type> file_timestamps;

        for (const auto& watch_path : watch_paths_) {
            for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
                if (fs::is_regular_file(entry.path())) {
                    file_timestamps[entry.path()] = fs::last_write_time(entry.path());
                }
            }
        }

        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            std::lock_guard<std::mutex> lock(mutex_);
            auto current_timestamps = file_timestamps;

            for (const auto& watch_path : watch_paths_) {
                if (!fs::exists(watch_path)) continue;

                for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
                    const auto& path = entry.path();

                    if (fs::is_regular_file(path)) {
                        auto current_time = fs::last_write_time(path);
                        auto it = current_timestamps.find(path);

                        if (it == current_timestamps.end()) {
                            file_timestamps[path] = current_time;
                            if (callback_) {
                                callback_(path, "CREATED");
                            }
                        } else if (it->second != current_time) {
                            file_timestamps[path] = current_time;
                            if (callback_) {
                                callback_(path, "MODIFIED");
                            }
                        }
                        current_timestamps.erase(path);
                    }
                }
            }

            for (const auto& [path, _] : current_timestamps) {
                file_timestamps.erase(path);
                if (callback_) {
                    callback_(path, "DELETED");
                }
            }
        }
    }

    std::vector<fs::path> watch_paths_;
    Callback callback_;
    std::atomic<bool> running_;
    std::thread monitor_thread_;
    std::mutex mutex_;
};

int main() {
    FileSystemWatcher watcher;

    watcher.setEventCallback([](const fs::path& path, const std::string& event) {
        std::cout << "[" << event << "] " << path.filename().string() << std::endl;
    });

    watcher.addWatchPath(".");
    watcher.start();

    std::cout << "File system watcher started. Press Enter to stop..." << std::endl;
    std::cin.get();

    watcher.stop();
    return 0;
}