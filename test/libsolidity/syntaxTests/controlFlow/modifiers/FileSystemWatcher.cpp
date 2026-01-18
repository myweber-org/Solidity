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
            watch_paths_.push_back(path);
            updateSnapshot(path);
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
    struct FileSnapshot {
        std::time_t last_write_time;
        std::uintmax_t file_size;
    };

    void updateSnapshot(const fs::path& directory) {
        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                auto ftime = fs::last_write_time(entry.path());
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
                std::time_t last_write = std::chrono::system_clock::to_time_t(sctp);

                snapshots_[entry.path()] = FileSnapshot{
                    last_write,
                    entry.file_size()
                };
            }
        }
    }

    void monitorLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            checkForChanges();
        }
    }

    void checkForChanges() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& directory : watch_paths_) {
            for (const auto& entry : fs::recursive_directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    const auto& path = entry.path();
                    auto current_ftime = fs::last_write_time(path);
                    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        current_ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
                    std::time_t current_last_write = std::chrono::system_clock::to_time_t(sctp);

                    auto it = snapshots_.find(path);
                    if (it == snapshots_.end()) {
                        snapshots_[path] = FileSnapshot{
                            current_last_write,
                            entry.file_size()
                        };
                        if (callback_) {
                            callback_(path, "CREATED");
                        }
                    } else {
                        if (it->second.last_write_time != current_last_write ||
                            it->second.file_size != entry.file_size()) {
                            it->second.last_write_time = current_last_write;
                            it->second.file_size = entry.file_size();
                            if (callback_) {
                                callback_(path, "MODIFIED");
                            }
                        }
                    }
                }
            }

            auto it = snapshots_.begin();
            while (it != snapshots_.end()) {
                if (!fs::exists(it->first)) {
                    auto path = it->first;
                    it = snapshots_.erase(it);
                    if (callback_) {
                        callback_(path, "DELETED");
                    }
                } else {
                    ++it;
                }
            }
        }
    }

    std::vector<fs::path> watch_paths_;
    std::unordered_map<fs::path, FileSnapshot> snapshots_;
    Callback callback_;
    std::thread monitor_thread_;
    std::atomic<bool> running_;
    std::mutex mutex_;
};

int main() {
    FileSystemWatcher watcher;
    
    watcher.setEventCallback([](const fs::path& path, const std::string& event) {
        std::cout << "Event: " << event << " | Path: " << path.string() << std::endl;
    });

    watcher.addWatchPath(fs::current_path());
    watcher.start();

    std::cout << "Watching directory: " << fs::current_path().string() << std::endl;
    std::cout << "Press Enter to stop watching..." << std::endl;
    
    std::cin.get();
    watcher.stop();
    
    return 0;
}