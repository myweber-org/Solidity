
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    SimpleFileWatcher(const std::string& path) : watch_path(path), running(false) {
        if (fs::exists(path) && fs::is_directory(path)) {
            build_snapshot();
        }
    }

    void start_watching(int interval_seconds = 1) {
        running = true;
        std::cout << "Watching directory: " << watch_path << std::endl;
        
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

    void stop_watching() {
        running = false;
    }

private:
    void build_snapshot() {
        snapshot.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                auto last_write = fs::last_write_time(entry.path());
                snapshot[entry.path().string()] = last_write;
            }
        }
    }

    void check_for_changes() {
        std::unordered_map<std::string, fs::file_time_type> current_snapshot;
        
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                auto path_str = entry.path().string();
                auto last_write = fs::last_write_time(entry.path());
                current_snapshot[path_str] = last_write;

                auto old_it = snapshot.find(path_str);
                if (old_it == snapshot.end()) {
                    std::cout << "[NEW] " << path_str << std::endl;
                } else if (old_it->second != last_write) {
                    std::cout << "[MODIFIED] " << path_str << std::endl;
                }
            }
        }

        for (const auto& old_entry : snapshot) {
            if (current_snapshot.find(old_entry.first) == current_snapshot.end()) {
                std::cout << "[DELETED] " << old_entry.first << std::endl;
            }
        }

        snapshot = std::move(current_snapshot);
    }

    std::string watch_path;
    std::unordered_map<std::string, fs::file_time_type> snapshot;
    bool running;
};

int main() {
    SimpleFileWatcher watcher(".");
    
    std::thread watch_thread([&watcher]() {
        watcher.start_watching(2);
    });

    std::cout << "File watcher started. Press Enter to stop..." << std::endl;
    std::cin.get();
    
    watcher.stop_watching();
    watch_thread.join();
    
    return 0;
}