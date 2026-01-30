
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    using FileTimeMap = std::unordered_map<std::string, fs::file_time_type>;

    SimpleFileWatcher(const std::string& path) : watch_path(path), running(false) {
        if (!fs::exists(path)) {
            throw std::runtime_error("Path does not exist: " + path);
        }
    }

    void start(int interval_seconds = 1) {
        running = true;
        snapshot_current_state();
        
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

    void stop() {
        running = false;
    }

private:
    std::string watch_path;
    bool running;
    FileTimeMap file_timestamps;

    void snapshot_current_state() {
        file_timestamps.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                file_timestamps[entry.path().string()] = fs::last_write_time(entry);
            }
        }
        std::cout << "Initial snapshot captured. Monitoring " << file_timestamps.size() << " files.\n";
    }

    void check_for_changes() {
        FileTimeMap current_state;
        
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                std::string path = entry.path().string();
                auto current_time = fs::last_write_time(entry);
                current_state[path] = current_time;

                auto it = file_timestamps.find(path);
                if (it == file_timestamps.end()) {
                    std::cout << "[NEW] " << path << std::endl;
                } else if (it->second != current_time) {
                    std::cout << "[MODIFIED] " << path << std::endl;
                }
            }
        }

        for (const auto& [path, _] : file_timestamps) {
            if (current_state.find(path) == current_state.end()) {
                std::cout << "[DELETED] " << path << std::endl;
            }
        }

        file_timestamps = std::move(current_state);
    }
};

int main() {
    try {
        SimpleFileWatcher watcher(".");
        std::cout << "Starting file system watcher. Press Ctrl+C to stop.\n";
        
        std::thread watch_thread([&watcher]() {
            watcher.start();
        });

        std::this_thread::sleep_for(std::chrono::seconds(30));
        watcher.stop();
        watch_thread.join();
        
        std::cout << "File watcher stopped.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}