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
            throw std::runtime_error("Directory does not exist or is not a directory");
        }
        populateFileMap();
    }

    void start() {
        running = true;
        watchThread = std::thread(&SimpleFileWatcher::watchLoop, this);
    }

    void stop() {
        running = false;
        if (watchThread.joinable()) {
            watchThread.join();
        }
    }

    ~SimpleFileWatcher() {
        stop();
    }

private:
    fs::path watch_directory;
    FileChangeCallback callback;
    std::unordered_map<std::string, fs::file_time_type> file_map;
    std::thread watchThread;
    bool running;

    void populateFileMap() {
        file_map.clear();
        for (const auto& entry : fs::directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.status())) {
                file_map[entry.path().filename().string()] = fs::last_write_time(entry);
            }
        }
    }

    void watchLoop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            std::vector<std::string> new_files;
            std::vector<std::string> modified_files;
            std::vector<std::string> deleted_files;

            std::unordered_map<std::string, fs::file_time_type> current_map;

            for (const auto& entry : fs::directory_iterator(watch_directory)) {
                if (fs::is_regular_file(entry.status())) {
                    std::string filename = entry.path().filename().string();
                    auto current_time = fs::last_write_time(entry);
                    current_map[filename] = current_time;

                    auto it = file_map.find(filename);
                    if (it == file_map.end()) {
                        new_files.push_back(filename);
                    } else if (it->second != current_time) {
                        modified_files.push_back(filename);
                    }
                }
            }

            for (const auto& [filename, _] : file_map) {
                if (current_map.find(filename) == current_map.end()) {
                    deleted_files.push_back(filename);
                }
            }

            for (const auto& filename : new_files) {
                callback(watch_directory / filename, "CREATED");
            }
            for (const auto& filename : modified_files) {
                callback(watch_directory / filename, "MODIFIED");
            }
            for (const auto& filename : deleted_files) {
                callback(watch_directory / filename, "DELETED");
            }

            file_map = std::move(current_map);
        }
    }
};

void exampleCallback(const fs::path& filepath, const std::string& change_type) {
    std::cout << "File: " << filepath << " - Change: " << change_type << std::endl;
}

int main() {
    try {
        SimpleFileWatcher watcher(fs::current_path(), exampleCallback);
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