
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>
#include <functional>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    using FileChangeCallback = std::function<void(const fs::path&, const std::string&)>;

    SimpleFileWatcher(const fs::path& watch_path, FileChangeCallback callback)
        : watch_path_(watch_path), callback_(callback), running_(false) {
        if (!fs::exists(watch_path_) || !fs::is_directory(watch_path_)) {
            throw std::runtime_error("Watch path does not exist or is not a directory");
        }
        build_file_map();
    }

    void start() {
        running_ = true;
        watcher_thread_ = std::thread(&SimpleFileWatcher::watch_loop, this);
    }

    void stop() {
        running_ = false;
        if (watcher_thread_.joinable()) {
            watcher_thread_.join();
        }
    }

    ~SimpleFileWatcher() {
        stop();
    }

private:
    void build_file_map() {
        file_map_.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (entry.is_regular_file()) {
                file_map_[entry.path()] = fs::last_write_time(entry);
            }
        }
    }

    void watch_loop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            std::unordered_map<fs::path, fs::file_time_type> current_map;
            for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
                if (entry.is_regular_file()) {
                    current_map[entry.path()] = fs::last_write_time(entry);
                }
            }

            for (const auto& [path, mtime] : current_map) {
                auto it = file_map_.find(path);
                if (it == file_map_.end()) {
                    callback_(path, "created");
                } else if (it->second != mtime) {
                    callback_(path, "modified");
                }
            }

            for (const auto& [path, mtime] : file_map_) {
                if (current_map.find(path) == current_map.end()) {
                    callback_(path, "deleted");
                }
            }

            file_map_.swap(current_map);
        }
    }

    fs::path watch_path_;
    FileChangeCallback callback_;
    std::unordered_map<fs::path, fs::file_time_type> file_map_;
    std::thread watcher_thread_;
    bool running_;
};

void example_callback(const fs::path& file_path, const std::string& change_type) {
    std::cout << "File: " << file_path.string() << " - Action: " << change_type << std::endl;
}

int main() {
    try {
        fs::path watch_dir = ".";
        SimpleFileWatcher watcher(watch_dir, example_callback);
        
        std::cout << "Watching directory: " << fs::absolute(watch_dir).string() << std::endl;
        std::cout << "Press Enter to stop watching..." << std::endl;
        
        watcher.start();
        std::cin.get();
        watcher.stop();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}