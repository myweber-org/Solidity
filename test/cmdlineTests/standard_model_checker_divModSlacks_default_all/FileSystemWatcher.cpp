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

    ~FileSystemWatcher() {
        stop();
    }

    void addWatchPath(const fs::path& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (fs::exists(path) && fs::is_directory(path)) {
            watch_paths_.insert(path);
            updateSnapshot(path);
        }
    }

    void setCallback(Callback cb) {
        callback_ = std::move(cb);
    }

    void start() {
        if (running_) return;
        
        running_ = true;
        monitor_thread_ = std::thread(&FileSystemWatcher::monitorLoop, this);
    }

    void stop() {
        running_ = false;
        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
    }

private:
    struct FileInfo {
        fs::file_time_type last_write_time;
        uintmax_t file_size;
    };

    void updateSnapshot(const fs::path& path) {
        auto& snapshot = snapshots_[path];
        snapshot.clear();

        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (entry.is_regular_file()) {
                try {
                    FileInfo info;
                    info.last_write_time = fs::last_write_time(entry);
                    info.file_size = entry.file_size();
                    snapshot[entry.path()] = info;
                } catch (const fs::filesystem_error&) {
                    continue;
                }
            }
        }
    }

    void checkForChanges() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (const auto& watch_path : watch_paths_) {
            auto& old_snapshot = snapshots_[watch_path];
            std::unordered_map<fs::path, FileInfo> new_snapshot;

            for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
                if (entry.is_regular_file()) {
                    try {
                        FileInfo info;
                        info.last_write_time = fs::last_write_time(entry);
                        info.file_size = entry.file_size();
                        new_snapshot[entry.path()] = info;

                        auto old_it = old_snapshot.find(entry.path());
                        if (old_it == old_snapshot.end()) {
                            notifyChange(entry.path(), "created");
                        } else if (info.last_write_time != old_it->second.last_write_time ||
                                   info.file_size != old_it->second.file_size) {
                            notifyChange(entry.path(), "modified");
                        }
                    } catch (const fs::filesystem_error&) {
                        continue;
                    }
                }
            }

            for (const auto& old_entry : old_snapshot) {
                if (new_snapshot.find(old_entry.first) == new_snapshot.end()) {
                    notifyChange(old_entry.first, "deleted");
                }
            }

            old_snapshot = std::move(new_snapshot);
        }
    }

    void notifyChange(const fs::path& path, const std::string& change_type) {
        if (callback_) {
            callback_(path, change_type);
        }
    }

    void monitorLoop() {
        while (running_) {
            checkForChanges();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    std::unordered_set<fs::path> watch_paths_;
    std::unordered_map<fs::path, std::unordered_map<fs::path, FileInfo>> snapshots_;
    Callback callback_;
    std::atomic<bool> running_;
    std::thread monitor_thread_;
    std::mutex mutex_;
};

int main() {
    FileSystemWatcher watcher;
    
    watcher.setCallback([](const fs::path& path, const std::string& change_type) {
        std::cout << "File " << path << " was " << change_type << std::endl;
    });

    watcher.addWatchPath(".");
    watcher.start();

    std::cout << "Watching for file changes. Press Enter to stop..." << std::endl;
    std::cin.get();
    
    watcher.stop();
    return 0;
}