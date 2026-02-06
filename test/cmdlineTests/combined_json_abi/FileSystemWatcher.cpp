
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    SimpleFileWatcher(const fs::path& path) : watch_path(path), running(false) {
        if (!fs::exists(path) || !fs::is_directory(path)) {
            throw std::runtime_error("Path does not exist or is not a directory");
        }
        snapshot();
    }

    void start(int interval_ms = 1000) {
        running = true;
        watcher_thread = std::thread([this, interval_ms]() {
            while (running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
                check_for_changes();
            }
        });
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
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps;
    std::thread watcher_thread;
    bool running;

    void snapshot() {
        file_timestamps.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_timestamps[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void check_for_changes() {
        std::unordered_map<std::string, fs::file_time_type> current_state;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                std::string path_str = entry.path().string();
                auto current_time = fs::last_write_time(entry.path());
                current_state[path_str] = current_time;

                auto it = file_timestamps.find(path_str);
                if (it == file_timestamps.end()) {
                    std::cout << "[CREATED] " << path_str << std::endl;
                } else if (it->second != current_time) {
                    std::cout << "[MODIFIED] " << path_str << std::endl;
                }
            }
        }

        for (const auto& [path, _] : file_timestamps) {
            if (current_state.find(path) == current_state.end()) {
                std::cout << "[DELETED] " << path << std::endl;
            }
        }

        file_timestamps.swap(current_state);
    }
};

int main() {
    try {
        SimpleFileWatcher watcher(".");
        std::cout << "Watching current directory for changes..." << std::endl;
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