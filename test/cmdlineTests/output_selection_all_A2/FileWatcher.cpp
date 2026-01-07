
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <iostream>
#include <functional>

namespace fs = std::filesystem;

class FileWatcher {
public:
    using FileChangeCallback = std::function<void(const fs::path&, bool)>;
    
    FileWatcher(const fs::path& directory, std::chrono::milliseconds interval)
        : watch_directory(directory), check_interval(interval), running(false) {
        
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            throw std::runtime_error("Invalid directory path");
        }
        
        initializeSnapshot();
    }
    
    void setCallback(FileChangeCallback callback) {
        change_callback = callback;
    }
    
    void start() {
        running = true;
        monitor_thread = std::thread(&FileWatcher::monitorLoop, this);
    }
    
    void stop() {
        running = false;
        if (monitor_thread.joinable()) {
            monitor_thread.join();
        }
    }
    
    ~FileWatcher() {
        stop();
    }

private:
    void initializeSnapshot() {
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.path())) {
                file_snapshot[entry.path()] = fs::last_write_time(entry.path());
            }
        }
    }
    
    void monitorLoop() {
        while (running) {
            std::this_thread::sleep_for(check_interval);
            checkForChanges();
        }
    }
    
    void checkForChanges() {
        std::unordered_map<fs::path, fs::file_time_type> current_snapshot;
        
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.path())) {
                current_snapshot[entry.path()] = fs::last_write_time(entry.path());
            }
        }
        
        for (const auto& [path, current_time] : current_snapshot) {
            auto it = file_snapshot.find(path);
            if (it == file_snapshot.end()) {
                file_snapshot[path] = current_time;
                if (change_callback) {
                    change_callback(path, true);
                }
            } else if (it->second != current_time) {
                it->second = current_time;
                if (change_callback) {
                    change_callback(path, false);
                }
            }
        }
        
        for (auto it = file_snapshot.begin(); it != file_snapshot.end();) {
            if (current_snapshot.find(it->first) == current_snapshot.end()) {
                if (change_callback) {
                    change_callback(it->first, false);
                }
                it = file_snapshot.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    fs::path watch_directory;
    std::chrono::milliseconds check_interval;
    std::unordered_map<fs::path, fs::file_time_type> file_snapshot;
    FileChangeCallback change_callback;
    std::thread monitor_thread;
    bool running;
};