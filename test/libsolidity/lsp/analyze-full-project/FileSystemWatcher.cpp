
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>
#include <functional>

namespace fs = std::filesystem;

class FileWatcher {
public:
    using FileChangeCallback = std::function<void(const fs::path&, const std::string&)>;

    FileWatcher(const fs::path& directory, FileChangeCallback callback)
        : watch_directory(directory), callback(callback), running(false) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            throw std::runtime_error("Invalid directory path");
        }
        initialize_file_map();
    }

    void start() {
        running = true;
        std::cout << "Starting file watcher for: " << watch_directory << std::endl;
        monitor_loop();
    }

    void stop() {
        running = false;
        std::cout << "File watcher stopped." << std::endl;
    }

private:
    fs::path watch_directory;
    FileChangeCallback callback;
    bool running;
    std::unordered_map<std::string, fs::file_time_type> file_map;

    void initialize_file_map() {
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.path())) {
                file_map[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void monitor_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            check_for_changes();
        }
    }

    void check_for_changes() {
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (!fs::is_regular_file(entry.path())) continue;

            auto current_path = entry.path().string();
            auto current_time = fs::last_write_time(entry.path());

            if (file_map.find(current_path) == file_map.end()) {
                file_map[current_path] = current_time;
                callback(entry.path(), "CREATED");
            } else if (file_map[current_path] != current_time) {
                file_map[current_path] = current_time;
                callback(entry.path(), "MODIFIED");
            }
        }

        check_for_deletions();
    }

    void check_for_deletions() {
        auto it = file_map.begin();
        while (it != file_map.end()) {
            if (!fs::exists(it->first)) {
                callback(fs::path(it->first), "DELETED");
                it = file_map.erase(it);
            } else {
                ++it;
            }
        }
    }
};

void example_callback(const fs::path& file_path, const std::string& change_type) {
    std::cout << "File: " << file_path.filename() 
              << " | Change: " << change_type 
              << " | Full path: " << file_path << std::endl;
}

int main() {
    try {
        fs::path current_dir = fs::current_path();
        FileWatcher watcher(current_dir, example_callback);
        
        std::thread watch_thread([&watcher]() {
            watcher.start();
        });

        std::cout << "File watcher is running. Press Enter to stop..." << std::endl;
        std::cin.get();
        
        watcher.stop();
        if (watch_thread.joinable()) {
            watch_thread.join();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}