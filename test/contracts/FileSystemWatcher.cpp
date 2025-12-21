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
        : watch_directory(directory), change_callback(callback), running(false) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            throw std::runtime_error("Directory does not exist or is not a directory.");
        }
        build_file_map();
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
    fs::path watch_directory;
    FileChangeCallback change_callback;
    std::thread watcher_thread;
    bool running;
    std::unordered_map<std::string, fs::file_time_type> file_modification_map;

    void build_file_map() {
        file_modification_map.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                auto path = entry.path();
                auto mod_time = fs::last_write_time(path);
                file_modification_map[path.string()] = mod_time;
            }
        }
    }

    void watch_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            std::unordered_map<std::string, fs::file_time_type> current_map;
            for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
                if (entry.is_regular_file()) {
                    auto path = entry.path();
                    auto mod_time = fs::last_write_time(path);
                    current_map[path.string()] = mod_time;
                }
            }

            for (const auto& [path, current_time] : current_map) {
                auto it = file_modification_map.find(path);
                if (it == file_modification_map.end()) {
                    change_callback(path, "CREATED");
                } else if (it->second != current_time) {
                    change_callback(path, "MODIFIED");
                }
            }

            for (const auto& [path, old_time] : file_modification_map) {
                if (current_map.find(path) == current_map.end()) {
                    change_callback(path, "DELETED");
                }
            }

            file_modification_map = std::move(current_map);
        }
    }
};

void example_callback(const fs::path& file_path, const std::string& change_type) {
    std::cout << "File: " << file_path.filename() << " - Action: " << change_type << std::endl;
}

int main() {
    try {
        SimpleFileWatcher watcher(fs::current_path(), example_callback);
        std::cout << "Watching directory: " << fs::current_path() << std::endl;
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