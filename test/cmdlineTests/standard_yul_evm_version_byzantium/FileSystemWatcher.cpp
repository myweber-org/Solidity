#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <functional>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const fs::file_time_type&)>;

    FileSystemWatcher(const fs::path& directory, Callback callback)
        : watch_directory(directory), on_modified(callback), running(false) {
        if (!fs::exists(watch_directory) || !fs::is_directory(watch_directory)) {
            throw std::runtime_error("Invalid directory path");
        }
        initialize_file_map();
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

    ~FileSystemWatcher() {
        stop();
    }

private:
    void initialize_file_map() {
        file_map.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.path())) {
                file_map[entry.path()] = fs::last_write_time(entry.path());
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
            if (!fs::is_regular_file(entry.path())) {
                continue;
            }

            auto current_time = fs::last_write_time(entry.path());
            auto it = file_map.find(entry.path());

            if (it == file_map.end()) {
                file_map[entry.path()] = current_time;
                if (on_modified) {
                    on_modified(entry.path(), current_time);
                }
            } else if (it->second != current_time) {
                it->second = current_time;
                if (on_modified) {
                    on_modified(entry.path(), current_time);
                }
            }
        }

        std::vector<fs::path> to_remove;
        for (const auto& [path, _] : file_map) {
            if (!fs::exists(path)) {
                to_remove.push_back(path);
            }
        }

        for (const auto& path : to_remove) {
            file_map.erase(path);
        }
    }

    fs::path watch_directory;
    Callback on_modified;
    std::unordered_map<fs::path, fs::file_time_type> file_map;
    std::thread monitor_thread;
    std::atomic<bool> running;
};

int main() {
    try {
        FileSystemWatcher watcher(fs::current_path(), [](const fs::path& path, const fs::file_time_type& time) {
            auto cftime = std::chrono::system_clock::to_time_t(
                std::chrono::file_clock::to_sys(time)
            );
            std::cout << "File modified: " << path.filename().string()
                      << " at " << std::ctime(&cftime);
        });

        watcher.start();
        std::cout << "Watching directory: " << fs::current_path().string() << std::endl;
        std::cout << "Press Enter to stop..." << std::endl;
        std::cin.get();
        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}