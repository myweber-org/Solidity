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
            watch_paths_.push_back(path);
            std::cout << "Watching directory: " << path.string() << std::endl;
        }
    }

    void setCallback(Callback cb) {
        callback_ = std::move(cb);
    }

    void start() {
        if (watch_paths_.empty() || !callback_) {
            std::cerr << "No watch paths or callback set." << std::endl;
            return;
        }

        running_ = true;
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
    void watchLoop() {
        std::unordered_map<std::string, fs::file_time_type> file_timestamps;

        for (const auto& dir : watch_paths_) {
            for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                if (fs::is_regular_file(entry.path())) {
                    file_timestamps[entry.path().string()] = fs::last_write_time(entry.path());
                }
            }
        }

        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            for (const auto& dir : watch_paths_) {
                for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                    if (!fs::is_regular_file(entry.path())) continue;

                    auto path_str = entry.path().string();
                    auto current_time = fs::last_write_time(entry.path());

                    if (file_timestamps.find(path_str) == file_timestamps.end()) {
                        file_timestamps[path_str] = current_time;
                        callback_(entry.path(), "CREATED");
                    } else if (file_timestamps[path_str] != current_time) {
                        file_timestamps[path_str] = current_time;
                        callback_(entry.path(), "MODIFIED");
                    }
                }
            }

            auto it = file_timestamps.begin();
            while (it != file_timestamps.end()) {
                if (!fs::exists(it->first)) {
                    callback_(fs::path(it->first), "DELETED");
                    it = file_timestamps.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    std::vector<fs::path> watch_paths_;
    Callback callback_;
    std::thread worker_thread_;
    std::atomic<bool> running_;
};

int main() {
    FileSystemWatcher watcher;

    watcher.setCallback([](const fs::path& path, const std::string& action) {
        std::cout << "[" << action << "] " << path.filename().string() << std::endl;
    });

    watcher.addWatchPath(".");
    watcher.start();

    std::this_thread::sleep_for(std::chrono::seconds(30));
    watcher.stop();

    return 0;
}