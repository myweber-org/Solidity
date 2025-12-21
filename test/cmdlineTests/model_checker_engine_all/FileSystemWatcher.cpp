
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& directory) : watch_directory(directory) {
        if (!fs::exists(watch_directory) || !fs::is_directory(watch_directory)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        populate_file_map();
    }

    void start_monitoring(int interval_seconds = 1) {
        std::cout << "Starting to monitor: " << watch_directory << std::endl;
        while (monitoring_active) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

    void stop_monitoring() {
        monitoring_active = false;
    }

private:
    fs::path watch_directory;
    std::unordered_map<std::string, fs::file_time_type> file_modification_map;
    bool monitoring_active{true};

    void populate_file_map() {
        file_modification_map.clear();
        for (const auto& entry : fs::directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.path())) {
                file_modification_map[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void check_for_changes() {
        for (const auto& entry : fs::directory_iterator(watch_directory)) {
            if (!fs::is_regular_file(entry.path())) {
                continue;
            }

            auto current_path = entry.path().string();
            auto current_write_time = fs::last_write_time(entry.path());

            if (file_modification_map.find(current_path) == file_modification_map.end()) {
                std::cout << "New file detected: " << current_path << std::endl;
                file_modification_map[current_path] = current_write_time;
            } else if (file_modification_map[current_path] != current_write_time) {
                std::cout << "File modified: " << current_path << std::endl;
                file_modification_map[current_path] = current_write_time;
            }
        }

        std::vector<std::string> files_to_remove;
        for (const auto& [path, time] : file_modification_map) {
            if (!fs::exists(path)) {
                std::cout << "File deleted: " << path << std::endl;
                files_to_remove.push_back(path);
            }
        }

        for (const auto& path : files_to_remove) {
            file_modification_map.erase(path);
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start_monitoring(2);
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}