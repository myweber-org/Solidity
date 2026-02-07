
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

    enum class EventType {
        Created,
        Modified,
        Deleted,
        Renamed
    };

    FileSystemWatcher() : running_(false) {}

    ~FileSystemWatcher() {
        stop();
    }

    void addWatchPath(const FilePath& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (fs::exists(path)) {
            auto ftime = fs::last_write_time(path);
            file_timestamps_[path] = ftime;
            watch_paths_.insert(path);
        }
    }

    void setEventCallback(Callback cb) {
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
    void monitorLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            std::lock_guard<std::mutex> lock(mutex_);
            
            for (const auto& path : watch_paths_) {
                if (!fs::exists(path)) {
                    if (file_timestamps_.count(path)) {
                        file_timestamps_.erase(path);
                        notifyEvent(path, "deleted");
                    }
                    continue;
                }

                auto current_time = fs::last_write_time(path);
                
                if (!file_timestamps_.count(path)) {
                    file_timestamps_[path] = current_time;
                    notifyEvent(path, "created");
                } else if (file_timestamps_[path] != current_time) {
                    file_timestamps_[path] = current_time;
                    notifyEvent(path, "modified");
                }
            }

            checkForNewFiles();
        }
    }

    void checkForNewFiles() {
        for (const auto& dir_path : watch_paths_) {
            if (fs::is_directory(dir_path)) {
                for (const auto& entry : fs::recursive_directory_iterator(dir_path)) {
                    FilePath path = entry.path();
                    if (!file_timestamps_.count(path)) {
                        file_timestamps_[path] = fs::last_write_time(path);
                        notifyEvent(path, "created");
                    }
                }
            }
        }
    }

    void notifyEvent(const FilePath& path, const std::string& event) {
        if (callback_) {
            callback_(path, event);
        }
    }

    std::unordered_map<FilePath, FileTime> file_timestamps_;
    std::unordered_set<FilePath> watch_paths_;
    Callback callback_;
    std::atomic<bool> running_;
    std::thread monitor_thread_;
    std::mutex mutex_;
};

int main() {
    FileSystemWatcher watcher;
    
    watcher.setEventCallback([](const FileSystemWatcher::FilePath& path, 
                                const std::string& event) {
        std::cout << "File: " << path.string() 
                  << " Event: " << event << std::endl;
    });

    watcher.addWatchPath(".");
    watcher.start();

    std::cout << "Watching current directory. Press Enter to stop..." << std::endl;
    std::cin.get();
    
    watcher.stop();
    return 0;
}