
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
    
    FileWatcher(const fs::path& directory, std::chrono::milliseconds interval = std::chrono::milliseconds(1000))
        : watch_directory(directory), check_interval(interval), running(false) {
        
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            throw std::runtime_error("Invalid directory path");
        }
    }
    
    void start(FileChangeCallback callback) {
        running = true;
        scan_existing_files();
        
        while (running) {
            std::this_thread::sleep_for(check_interval);
            check_for_changes(callback);
        }
    }
    
    void stop() {
        running = false;
    }
    
private:
    void scan_existing_files() {
        file_timestamps.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.path())) {
                file_timestamps[entry.path()] = fs::last_write_time(entry.path());
            }
        }
    }
    
    void check_for_changes(FileChangeCallback callback) {
        std::unordered_map<fs::path, fs::file_time_type> current_timestamps;
        
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.path())) {
                current_timestamps[entry.path()] = fs::last_write_time(entry.path());
            }
        }
        
        for (const auto& [path, current_time] : current_timestamps) {
            auto it = file_timestamps.find(path);
            if (it == file_timestamps.end()) {
                callback(path, true);
            } else if (it->second != current_time) {
                callback(path, false);
            }
        }
        
        for (const auto& [path, old_time] : file_timestamps) {
            if (current_timestamps.find(path) == current_timestamps.end()) {
                callback(path, false);
            }
        }
        
        file_timestamps = std::move(current_timestamps);
    }
    
    fs::path watch_directory;
    std::chrono::milliseconds check_interval;
    bool running;
    std::unordered_map<fs::path, fs::file_time_type> file_timestamps;
};