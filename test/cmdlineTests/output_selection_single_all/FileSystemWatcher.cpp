#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <functional>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/inotify.h>
#include <unistd.h>
#include <errno.h>
#endif

class FileSystemWatcher {
public:
    using Callback = std::function<void(const std::string&, uint32_t)>;
    
    enum Event {
        CREATED = 0x01,
        DELETED = 0x02,
        MODIFIED = 0x04,
        RENAMED = 0x08
    };
    
    FileSystemWatcher() : running_(false) {}
    
    ~FileSystemWatcher() {
        stop();
    }
    
    bool addWatch(const std::string& path, uint32_t events, Callback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!std::filesystem::exists(path)) {
            std::cerr << "Path does not exist: " << path << std::endl;
            return false;
        }
        
        WatchInfo info;
        info.path = std::filesystem::absolute(path).string();
        info.events = events;
        info.callback = callback;
        
        watches_.push_back(info);
        return true;
    }
    
    bool start() {
        if (running_) return false;
        
        running_ = true;
        watch_thread_ = std::thread(&FileSystemWatcher::watchLoop, this);
        return true;
    }
    
    void stop() {
        running_ = false;
        if (watch_thread_.joinable()) {
            watch_thread_.join();
        }
    }
    
private:
    struct WatchInfo {
        std::string path;
        uint32_t events;
        Callback callback;
    };
    
    void watchLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            std::lock_guard<std::mutex> lock(mutex_);
            
            for (const auto& watch : watches_) {
                checkPathChanges(watch);
            }
        }
    }
    
    void checkPathChanges(const WatchInfo& watch) {
        static std::unordered_map<std::string, std::filesystem::file_time_type> lastModified;
        
        try {
            if (watch.events & DELETED) {
                if (!std::filesystem::exists(watch.path)) {
                    auto it = lastModified.find(watch.path);
                    if (it != lastModified.end()) {
                        watch.callback(watch.path, DELETED);
                        lastModified.erase(it);
                    }
                }
            }
            
            if (std::filesystem::exists(watch.path)) {
                auto currentTime = std::filesystem::last_write_time(watch.path);
                auto it = lastModified.find(watch.path);
                
                if (it == lastModified.end()) {
                    if (watch.events & CREATED) {
                        watch.callback(watch.path, CREATED);
                    }
                    lastModified[watch.path] = currentTime;
                } else if (it->second != currentTime) {
                    if (watch.events & MODIFIED) {
                        watch.callback(watch.path, MODIFIED);
                    }
                    it->second = currentTime;
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error checking path: " << e.what() << std::endl;
        }
    }
    
    std::vector<WatchInfo> watches_;
    std::thread watch_thread_;
    std::mutex mutex_;
    std::atomic<bool> running_;
};

int main() {
    FileSystemWatcher watcher;
    
    watcher.addWatch(".", 
        FileSystemWatcher::CREATED | FileSystemWatcher::MODIFIED | FileSystemWatcher::DELETED,
        [](const std::string& path, uint32_t event) {
            std::string eventStr;
            if (event & FileSystemWatcher::CREATED) eventStr = "CREATED";
            if (event & FileSystemWatcher::MODIFIED) eventStr = "MODIFIED";
            if (event & FileSystemWatcher::DELETED) eventStr = "DELETED";
            
            std::cout << "Event: " << eventStr << " Path: " << path << std::endl;
        }
    );
    
    std::cout << "Watching current directory. Press Enter to stop..." << std::endl;
    watcher.start();
    
    std::cin.get();
    watcher.stop();
    
    return 0;
}