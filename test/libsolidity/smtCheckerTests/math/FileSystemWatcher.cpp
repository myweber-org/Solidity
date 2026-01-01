
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
    using FileChangeCallback = std::function<void(const fs::path&, const fs::file_time_type&)>;

    FileSystemWatcher() : running_(false) {}

    void watchDirectory(const fs::path& directory, FileChangeCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            throw std::runtime_error("Invalid directory path");
        }

        watch_list_[directory] = {callback, getCurrentFileTimes(directory)};
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
    struct WatchEntry {
        FileChangeCallback callback;
        std::unordered_map<std::string, fs::file_time_type> file_times;
    };

    std::unordered_map<std::string, WatchEntry> watch_list_;
    std::atomic<bool> running_;
    std::thread monitor_thread_;
    std::mutex mutex_;

    std::unordered_map<std::string, fs::file_time_type> getCurrentFileTimes(const fs::path& directory) {
        std::unordered_map<std::string, fs::file_time_type> times;
        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (fs::is_regular_file(entry.path())) {
                times[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
        return times;
    }

    void monitorLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            std::lock_guard<std::mutex> lock(mutex_);
            for (auto& [dir_path, entry] : watch_list_) {
                auto current_times = getCurrentFileTimes(dir_path);

                for (const auto& [file_path, current_time] : current_times) {
                    auto it = entry.file_times.find(file_path);
                    if (it == entry.file_times.end()) {
                        entry.callback(file_path, current_time);
                        entry.file_times[file_path] = current_time;
                    } else if (it->second != current_time) {
                        entry.callback(file_path, current_time);
                        it->second = current_time;
                    }
                }

                for (auto it = entry.file_times.begin(); it != entry.file_times.end();) {
                    if (current_times.find(it->first) == current_times.end()) {
                        entry.callback(it->first, fs::file_time_type::min());
                        it = entry.file_times.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
        }
    }
};

void sampleCallback(const fs::path& file_path, const fs::file_time_type& mod_time) {
    if (mod_time == fs::file_time_type::min()) {
        std::cout << "File deleted: " << file_path << std::endl;
    } else {
        std::cout << "File modified: " << file_path 
                  << " at " << mod_time.time_since_epoch().count() << std::endl;
    }
}

int main() {
    try {
        FileSystemWatcher watcher;
        watcher.watchDirectory(".", sampleCallback);
        watcher.start();

        std::cout << "Watching current directory. Press Enter to stop..." << std::endl;
        std::cin.get();

        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}