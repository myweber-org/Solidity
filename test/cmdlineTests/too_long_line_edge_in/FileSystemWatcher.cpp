
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <vector>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher() : running_(false) {}

    void addWatchPath(const fs::path& path) {
        if (fs::exists(path) && fs::is_directory(path)) {
            watch_paths_.push_back(fs::canonical(path));
        }
    }

    void setEventCallback(Callback callback) {
        callback_ = std::move(callback);
    }

    void start() {
        if (running_) return;

        running_ = true;
        snapshotCurrentState();
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
    void snapshotCurrentState() {
        file_states_.clear();
        for (const auto& watch_path : watch_paths_) {
            for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
                if (entry.is_regular_file()) {
                    auto last_write = fs::last_write_time(entry);
                    file_states_[entry.path()] = last_write;
                }
            }
        }
    }

    void monitorLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            std::unordered_map<fs::path, fs::file_time_type> current_states;

            for (const auto& watch_path : watch_paths_) {
                try {
                    for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
                        if (entry.is_regular_file()) {
                            auto path = entry.path();
                            auto last_write = fs::last_write_time(entry);
                            current_states[path] = last_write;

                            auto it = file_states_.find(path);
                            if (it == file_states_.end()) {
                                if (callback_) callback_(path, "created");
                            } else if (it->second != last_write) {
                                if (callback_) callback_(path, "modified");
                            }
                        }
                    }
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Filesystem error: " << e.what() << std::endl;
                }
            }

            for (const auto& [path, _] : file_states_) {
                if (current_states.find(path) == current_states.end()) {
                    if (callback_) callback_(path, "deleted");
                }
            }

            file_states_.swap(current_states);
        }
    }

    std::vector<fs::path> watch_paths_;
    std::unordered_map<fs::path, fs::file_time_type> file_states_;
    Callback callback_;
    std::thread monitor_thread_;
    std::atomic<bool> running_;
};

void exampleUsage() {
    FileSystemWatcher watcher;
    
    watcher.addWatchPath(".");
    watcher.addWatchPath("../parent_dir");

    watcher.setEventCallback([](const fs::path& path, const std::string& action) {
        std::cout << "File: " << path << " Action: " << action << std::endl;
    });

    watcher.start();

    std::this_thread::sleep_for(std::chrono::seconds(30));
    watcher.stop();
}

int main() {
    exampleUsage();
    return 0;
}