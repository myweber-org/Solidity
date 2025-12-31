
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>
#include <functional>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    using FileChangeCallback = std::function<void(const fs::path&, const std::string&)>;

    SimpleFileWatcher(const fs::path& watch_path, FileChangeCallback callback)
        : watch_path_(watch_path), callback_(callback), running_(false) {
        if (!fs::exists(watch_path_)) {
            throw std::runtime_error("Watch path does not exist");
        }
        scanCurrentState();
    }

    ~SimpleFileWatcher() {
        stop();
    }

    void start() {
        running_ = true;
        watcher_thread_ = std::thread(&SimpleFileWatcher::watchLoop, this);
    }

    void stop() {
        running_ = false;
        if (watcher_thread_.joinable()) {
            watcher_thread_.join();
        }
    }

private:
    void scanCurrentState() {
        file_states_.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (entry.is_regular_file()) {
                auto last_write = fs::last_write_time(entry.path());
                file_states_[entry.path()] = last_write;
            }
        }
    }

    void watchLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            std::unordered_map<fs::path, fs::file_time_type> current_states;

            for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
                if (entry.is_regular_file()) {
                    auto path = entry.path();
                    auto last_write = fs::last_write_time(path);
                    current_states[path] = last_write;

                    auto it = file_states_.find(path);
                    if (it == file_states_.end()) {
                        if (callback_) {
                            callback_(path, "created");
                        }
                    } else if (it->second != last_write) {
                        if (callback_) {
                            callback_(path, "modified");
                        }
                    }
                }
            }

            for (const auto& [path, _] : file_states_) {
                if (current_states.find(path) == current_states.end()) {
                    if (callback_) {
                        callback_(path, "deleted");
                    }
                }
            }

            file_states_.swap(current_states);
        }
    }

    fs::path watch_path_;
    FileChangeCallback callback_;
    std::unordered_map<fs::path, fs::file_time_type> file_states_;
    std::thread watcher_thread_;
    std::atomic<bool> running_;
};

void exampleCallback(const fs::path& path, const std::string& change_type) {
    std::cout << "File: " << path.string() << " - " << change_type << std::endl;
}

int main() {
    try {
        SimpleFileWatcher watcher(".", exampleCallback);
        watcher.start();

        std::cout << "Watching current directory. Press Enter to stop..." << std::endl;
        std::cin.get();

        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}#include <iostream>
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
                            data.callback(entry_path, "created");
                            tracked_files_.insert(entry_path);
                        }
                    }
                }

                auto it = tracked_files_.begin();
                while (it != tracked_files_.end()) {
                    if (!fs::exists(*it)) {
                        data.callback(*it, "deleted");
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
    std::unordered_set<fs::path> tracked_files_;
    std::atomic<bool> running_;
    std::thread monitor_thread_;
    std::mutex mutex_;
};

int main() {
    FileSystemWatcher watcher;

    watcher.addWatchPath("./logs", [](const fs::path& path, const std::string& action) {
        std::cout << "File " << path << " has been " << action << std::endl;
    });

    watcher.addWatchPath("./config", [](const fs::path& path, const std::string& action) {
        std::cerr << "Config change detected: " << path << " -> " << action << std::endl;
    });

    watcher.start();

    std::this_thread::sleep_for(std::chrono::seconds(30));
    watcher.stop();

    return 0;
}