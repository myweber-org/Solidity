
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <iostream>
#include <functional>
#include <atomic>
#include <mutex>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;
    
    enum class EventType {
        CREATED,
        MODIFIED,
        DELETED,
        RENAMED
    };
    
    FileSystemWatcher() : running_(false) {}
    
    ~FileSystemWatcher() {
        stop();
    }
    
    void addWatchPath(const fs::path& path, bool recursive = true) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (fs::exists(path) && fs::is_directory(path)) {
            watch_paths_.push_back({path, recursive});
            scanDirectory(path, recursive);
        }
    }
    
    void setEventCallback(EventType event, Callback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        callbacks_[static_cast<int>(event)] = std::move(callback);
    }
    
    void start(int interval_ms = 1000) {
        if (running_) return;
        
        running_ = true;
        monitor_thread_ = std::thread([this, interval_ms]() {
            while (running_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
                checkForChanges();
            }
        });
    }
    
    void stop() {
        running_ = false;
        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
    }
    
private:
    struct WatchInfo {
        fs::path path;
        bool recursive;
    };
    
    struct FileInfo {
        std::uintmax_t size;
        std::time_t last_write_time;
        fs::file_type type;
    };
    
    void scanDirectory(const fs::path& path, bool recursive) {
        try {
            for (const auto& entry : fs::directory_iterator(path)) {
                const auto& current_path = entry.path();
                FileInfo info{
                    entry.is_regular_file() ? entry.file_size() : 0,
                    fs::last_write_time(entry).time_since_epoch().count(),
                    entry.status().type()
                };
                
                file_cache_[current_path] = info;
                
                if (recursive && entry.is_directory()) {
                    scanDirectory(current_path, true);
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
    }
    
    void checkForChanges() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (const auto& watch_info : watch_paths_) {
            checkDirectory(watch_info.path, watch_info.recursive);
        }
        
        checkForDeletedFiles();
    }
    
    void checkDirectory(const fs::path& path, bool recursive) {
        try {
            for (const auto& entry : fs::directory_iterator(path)) {
                const auto& current_path = entry.path();
                
                if (!file_cache_.count(current_path)) {
                    handleFileEvent(current_path, EventType::CREATED);
                    
                    FileInfo info{
                        entry.is_regular_file() ? entry.file_size() : 0,
                        fs::last_write_time(entry).time_since_epoch().count(),
                        entry.status().type()
                    };
                    file_cache_[current_path] = info;
                } else {
                    auto& cached_info = file_cache_[current_path];
                    FileInfo current_info{
                        entry.is_regular_file() ? entry.file_size() : 0,
                        fs::last_write_time(entry).time_since_epoch().count(),
                        entry.status().type()
                    };
                    
                    if (cached_info.last_write_time != current_info.last_write_time ||
                        cached_info.size != current_info.size) {
                        handleFileEvent(current_path, EventType::MODIFIED);
                        cached_info = current_info;
                    }
                }
                
                if (recursive && entry.is_directory()) {
                    checkDirectory(current_path, true);
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
    }
    
    void checkForDeletedFiles() {
        auto it = file_cache_.begin();
        while (it != file_cache_.end()) {
            if (!fs::exists(it->first)) {
                handleFileEvent(it->first, EventType::DELETED);
                it = file_cache_.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    void handleFileEvent(const fs::path& path, EventType event) {
        auto callback_it = callbacks_.find(static_cast<int>(event));
        if (callback_it != callbacks_.end() && callback_it->second) {
            std::string event_str;
            switch (event) {
                case EventType::CREATED: event_str = "CREATED"; break;
                case EventType::MODIFIED: event_str = "MODIFIED"; break;
                case EventType::DELETED: event_str = "DELETED"; break;
                case EventType::RENAMED: event_str = "RENAMED"; break;
            }
            callback_it->second(path, event_str);
        }
    }
    
    std::vector<WatchInfo> watch_paths_;
    std::unordered_map<fs::path, FileInfo> file_cache_;
    std::unordered_map<int, Callback> callbacks_;
    std::thread monitor_thread_;
    std::atomic<bool> running_;
    std::mutex mutex_;
};

int main() {
    FileSystemWatcher watcher;
    
    watcher.setEventCallback(FileSystemWatcher::EventType::CREATED, 
        [](const fs::path& path, const std::string& event) {
            std::cout << event << ": " << path << std::endl;
        });
    
    watcher.setEventCallback(FileSystemWatcher::EventType::MODIFIED,
        [](const fs::path& path, const std::string& event) {
            std::cout << event << ": " << path << std::endl;
        });
    
    watcher.setEventCallback(FileSystemWatcher::EventType::DELETED,
        [](const fs::path& path, const std::string& event) {
            std::cout << event << ": " << path << std::endl;
        });
    
    watcher.addWatchPath(fs::current_path());
    watcher.start();
    
    std::cout << "Watching directory: " << fs::current_path() << std::endl;
    std::cout << "Press Enter to stop..." << std::endl;
    
    std::cin.get();
    watcher.stop();
    
    return 0;
}