#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>
#include <functional>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher(const fs::path& directory, Callback callback)
        : watch_directory(directory), notify_callback(callback), running(false) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        cache_current_state();
    }

    ~FileSystemWatcher() {
        stop();
    }

    void start() {
        running = true;
        monitor_thread = std::thread(&FileSystemWatcher::monitor_loop, this);
    }

    void stop() {
        running = false;
        if (monitor_thread.joinable()) {
            monitor_thread.join();
        }
    }

private:
    fs::path watch_directory;
    Callback notify_callback;
    std::unordered_map<std::string, fs::file_time_type> file_cache;
    std::thread monitor_thread;
    bool running;

    void cache_current_state() {
        file_cache.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.status())) {
                file_cache[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void monitor_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            auto current_state = file_cache;
            cache_current_state();

            for (const auto& [path, old_time] : current_state) {
                if (file_cache.find(path) == file_cache.end()) {
                    notify_callback(path, "deleted");
                } else if (file_cache[path] != old_time) {
                    notify_callback(path, "modified");
                }
            }

            for (const auto& [path, new_time] : file_cache) {
                if (current_state.find(path) == current_state.end()) {
                    notify_callback(path, "created");
                }
            }
        }
    }
};

void example_callback(const fs::path& file_path, const std::string& action) {
    std::cout << "File: " << file_path.filename() << " Action: " << action << std::endl;
}

int main() {
    try {
        fs::path watch_path = fs::current_path() / "watch_folder";
        fs::create_directory(watch_path);

        FileSystemWatcher watcher(watch_path, example_callback);
        watcher.start();

        std::cout << "Watching directory: " << watch_path << std::endl;
        std::cout << "Press Enter to stop..." << std::endl;
        std::cin.get();

        watcher.stop();
        fs::remove_all(watch_path);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}