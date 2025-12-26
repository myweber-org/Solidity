
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    SimpleFileWatcher(const fs::path& path, std::chrono::milliseconds interval)
        : watch_path(path), poll_interval(interval), running(false) {
        if (!fs::exists(watch_path)) {
            throw std::runtime_error("Path does not exist: " + watch_path.string());
        }
        build_snapshot();
    }

    void start() {
        running = true;
        watcher_thread = std::thread(&SimpleFileWatcher::watch_loop, this);
    }

    void stop() {
        running = false;
        if (watcher_thread.joinable()) {
            watcher_thread.join();
        }
    }

    ~SimpleFileWatcher() {
        stop();
    }

private:
    void build_snapshot() {
        file_snapshot.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                auto last_write = fs::last_write_time(entry);
                file_snapshot[entry.path()] = last_write;
            }
        }
    }

    void watch_loop() {
        while (running) {
            std::this_thread::sleep_for(poll_interval);
            check_for_changes();
        }
    }

    void check_for_changes() {
        std::unordered_map<fs::path, fs::file_time_type> current_snapshot;
        
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                auto last_write = fs::last_write_time(entry);
                current_snapshot[entry.path()] = last_write;
                
                auto it = file_snapshot.find(entry.path());
                if (it == file_snapshot.end()) {
                    std::cout << "[CREATED] " << entry.path() << std::endl;
                } else if (it->second != last_write) {
                    std::cout << "[MODIFIED] " << entry.path() << std::endl;
                }
            }
        }
        
        for (const auto& [path, _] : file_snapshot) {
            if (current_snapshot.find(path) == current_snapshot.end()) {
                std::cout << "[DELETED] " << path << std::endl;
            }
        }
        
        file_snapshot = std::move(current_snapshot);
    }

    fs::path watch_path;
    std::chrono::milliseconds poll_interval;
    std::unordered_map<fs::path, fs::file_time_type> file_snapshot;
    std::thread watcher_thread;
    std::atomic<bool> running;
};

int main() {
    try {
        SimpleFileWatcher watcher(fs::current_path(), std::chrono::seconds(2));
        watcher.start();
        
        std::cout << "Watching directory: " << fs::current_path() << std::endl;
        std::cout << "Press Enter to stop..." << std::endl;
        std::cin.get();
        
        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}