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

    ~SimpleFileWatcher() {
        stop();
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

private:
    fs::path watch_directory;
    FileChangeCallback change_callback;
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

    void watch_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            std::vector<std::string> created_files, modified_files, deleted_files;

            std::unordered_map<std::string, fs::file_time_type> current_file_map;
            for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
                if (entry.is_regular_file()) {
                    std::string path_str = entry.path().string();
                    auto current_time = fs::last_write_time(entry);
                    current_file_map[path_str] = current_time;

                    auto it = file_map.find(path_str);
                    if (it == file_map.end()) {
                        created_files.push_back(path_str);
                    } else if (it->second != current_time) {
                        modified_files.push_back(path_str);
                    }
                }
            }

            for (const auto& [path, _] : file_map) {
                if (current_file_map.find(path) == current_file_map.end()) {
                    deleted_files.push_back(path);
                }
            }

            file_map.swap(current_file_map);

            for (const auto& path : created_files) {
                if (change_callback) {
                    change_callback(path, "CREATED");
                }
            }
            for (const auto& path : modified_files) {
                if (change_callback) {
                    change_callback(path, "MODIFIED");
                }
            }
            for (const auto& path : deleted_files) {
                if (change_callback) {
                    change_callback(path, "DELETED");
                }
            }
        }
    }
};

void example_callback(const fs::path& file_path, const std::string& change_type) {
    std::cout << "File: " << file_path << " - Change: " << change_type << std::endl;
}

int main() {
    try {
        fs::path current_dir = fs::current_path();
        SimpleFileWatcher watcher(current_dir, example_callback);

        std::cout << "Watching directory: " << current_dir << std::endl;
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