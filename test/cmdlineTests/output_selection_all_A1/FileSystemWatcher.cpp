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

    SimpleFileWatcher(const fs::path& directory, FileChangeCallback callback)
        : watch_directory(directory), callback(callback), running(false) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            throw std::runtime_error("Directory does not exist or is not a directory.");
        }
        build_file_map();
    }

    ~SimpleFileWatcher() {
        stop();
    }

    void start(int interval_ms = 1000) {
        running = true;
        watcher_thread = std::thread(&SimpleFileWatcher::watch_loop, this, interval_ms);
    }

    void stop() {
        running = false;
        if (watcher_thread.joinable()) {
            watcher_thread.join();
        }
    }

private:
    fs::path watch_directory;
    FileChangeCallback callback;
    std::unordered_map<std::string, fs::file_time_type> file_map;
    std::thread watcher_thread;
    bool running;

    void build_file_map() {
        file_map.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                file_map[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void watch_loop(int interval_ms) {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
            check_for_changes();
        }
    }

    void check_for_changes() {
        std::unordered_map<std::string, fs::file_time_type> current_state;

        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                std::string path_str = entry.path().string();
                current_state[path_str] = fs::last_write_time(entry);
            }
        }

        for (const auto& [path, current_time] : current_state) {
            auto it = file_map.find(path);
            if (it == file_map.end()) {
                callback(path, "CREATED");
            } else if (it->second != current_time) {
                callback(path, "MODIFIED");
            }
        }

        for (const auto& [path, old_time] : file_map) {
            if (current_state.find(path) == current_state.end()) {
                callback(path, "DELETED");
            }
        }

        file_map = std::move(current_state);
    }
};

void example_callback(const fs::path& file_path, const std::string& change_type) {
    std::cout << "File: " << file_path << " - Change: " << change_type << std::endl;
}

int main() {
    try {
        fs::path watch_path = fs::current_path();
        SimpleFileWatcher watcher(watch_path, example_callback);

        std::cout << "Watching directory: " << watch_path << std::endl;
        std::cout << "Press Enter to stop watching..." << std::endl;

        watcher.start(2000);

        std::cin.get();
        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}