
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
    using FilePath = fs::path;
    using FileTime = fs::file_time_type;
    using Callback = std::function<void(const FilePath&, const std::string&)>;

    FileSystemWatcher() : running_(false) {}

    void addWatchPath(const FilePath& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (fs::exists(path)) {
            file_timestamps_[path] = fs::last_write_time(path);
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
        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            checkFileChanges();
        }
    }

    void checkFileChanges() {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = file_timestamps_.begin();
        while (it != file_timestamps_.end()) {
            const auto& path = it->first;
            if (!fs::exists(path)) {
                if (callback_) {
                    callback_(path, "deleted");
                }
                it = file_timestamps_.erase(it);
                continue;
            }

            auto current_time = fs::last_write_time(path);
            if (current_time != it->second) {
                if (callback_) {
                    callback_(path, "modified");
                }
                it->second = current_time;
            }
            ++it;
        }
    }

    std::unordered_map<FilePath, FileTime> file_timestamps_;
    Callback callback_;
    std::atomic<bool> running_;
    std::thread monitor_thread_;
    std::mutex mutex_;
};

int main() {
    FileSystemWatcher watcher;
    
    watcher.setEventCallback([](const FileSystemWatcher::FilePath& path, const std::string& event) {
        std::cout << "File: " << path.string() << " Event: " << event << std::endl;
    });

    watcher.addWatchPath("test_file.txt");
    watcher.start();

    std::cout << "Watching for file changes. Press Enter to stop..." << std::endl;
    std::cin.get();

    watcher.stop();
    return 0;
}