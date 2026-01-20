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
            watch_paths_[path] = {callback, getCurrentFileState(path)};
        }
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
        std::unordered_map<std::string, fs::file_time_type> file_states;
    };

    std::unordered_map<fs::path, WatchInfo> watch_paths_;
    std::atomic<bool> running_;
    std::thread monitor_thread_;
    std::mutex mutex_;

    std::unordered_map<std::string, fs::file_time_type> getCurrentFileState(const fs::path& directory) {
        std::unordered_map<std::string, fs::file_time_type> states;
        try {
            for (const auto& entry : fs::recursive_directory_iterator(directory)) {
                if (fs::is_regular_file(entry.status())) {
                    states[entry.path().string()] = fs::last_write_time(entry);
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
        return states;
    }

    void monitorLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::lock_guard<std::mutex> lock(mutex_);

            for (auto& [path, info] : watch_paths_) {
                auto current_states = getCurrentFileState(path);

                for (const auto& [file_path, current_time] : current_states) {
                    auto it = info.file_states.find(file_path);
                    if (it == info.file_states.end()) {
                        info.callback(file_path, "created");
                    } else if (it->second != current_time) {
                        info.callback(file_path, "modified");
                    }
                }

                for (const auto& [file_path, old_time] : info.file_states) {
                    if (current_states.find(file_path) == current_states.end()) {
                        info.callback(file_path, "deleted");
                    }
                }

                info.file_states = std::move(current_states);
            }
        }
    }
};

void exampleCallback(const fs::path& file_path, const std::string& action) {
    std::cout << "File: " << file_path << " Action: " << action << std::endl;
}

int main() {
    FileSystemWatcher watcher;

    watcher.addWatchPath("./test_directory", exampleCallback);
    watcher.addWatchPath("./another_directory", [](const fs::path& p, const std::string& a) {
        std::cout << "[Custom] " << p.filename() << " -> " << a << std::endl;
    });

    watcher.start();

    std::this_thread::sleep_for(std::chrono::seconds(30));

    watcher.stop();
    return 0;
}