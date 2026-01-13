
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
    using Callback = std::function<void(const fs::path&, const fs::file_time_type&)>;

    FileSystemWatcher() : running_(false) {}

    void addWatchPath(const fs::path& path, Callback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (fs::exists(path)) {
            auto lastWriteTime = fs::last_write_time(path);
            watchMap_[path] = {callback, lastWriteTime};
            std::cout << "Watching: " << path.string() << std::endl;
        }
    }

    void start() {
        running_ = true;
        monitorThread_ = std::thread(&FileSystemWatcher::monitorLoop, this);
    }

    void stop() {
        running_ = false;
        if (monitorThread_.joinable()) {
            monitorThread_.join();
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

private:
    void monitorLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            std::lock_guard<std::mutex> lock(mutex_);
            for (auto& [path, data] : watchMap_) {
                if (!fs::exists(path)) {
                    continue;
                }

                auto currentWriteTime = fs::last_write_time(path);
                if (data.lastWriteTime != currentWriteTime) {
                    data.callback(path, currentWriteTime);
                    data.lastWriteTime = currentWriteTime;
                }
            }
        }
    }

    struct WatchData {
        Callback callback;
        fs::file_time_type lastWriteTime;
    };

    std::unordered_map<fs::path, WatchData> watchMap_;
    std::thread monitorThread_;
    std::atomic<bool> running_;
    std::mutex mutex_;
};

void exampleCallback(const fs::path& path, const fs::file_time_type& modTime) {
    auto timeT = std::chrono::system_clock::to_time_t(
        std::chrono::file_clock::to_sys(modTime)
    );
    std::cout << "File modified: " << path.filename().string()
              << " at " << std::ctime(&timeT);
}

int main() {
    FileSystemWatcher watcher;

    watcher.addWatchPath("config.json", exampleCallback);
    watcher.addWatchPath("data.log", [](const fs::path& p, auto) {
        std::cout << "Log file updated: " << p.string() << std::endl;
    });

    watcher.start();

    std::cout << "File system watcher started. Press Enter to stop..." << std::endl;
    std::cin.get();

    watcher.stop();
    return 0;
}