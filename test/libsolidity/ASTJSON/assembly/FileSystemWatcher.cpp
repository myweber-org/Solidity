
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& directory) : watch_directory(directory) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            throw std::invalid_argument("Provided path is not a valid directory");
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
    std::unordered_map<std::string, fs::file_time_type> file_map;
    bool monitoring_active = true;

    void populate_file_map() {
        file_map.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.path())) {
                file_map[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void check_for_changes() {
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (!fs::is_regular_file(entry.path())) {
                continue;
            }

            auto current_path = entry.path().string();
            auto current_time = fs::last_write_time(entry.path());

            if (file_map.find(current_path) == file_map.end()) {
                std::cout << "[NEW] File created: " << current_path << std::endl;
                file_map[current_path] = current_time;
            } else if (file_map[current_path] != current_time) {
                std::cout << "[MODIFIED] File changed: " << current_path << std::endl;
                file_map[current_path] = current_time;
            }
        }

        std::vector<std::string> files_to_remove;
        for (const auto& [file_path, _] : file_map) {
            if (!fs::exists(file_path)) {
                std::cout << "[DELETED] File removed: " << file_path << std::endl;
                files_to_remove.push_back(file_path);
            }
        }

        for (const auto& file_path : files_to_remove) {
            file_map.erase(file_path);
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start_monitoring(2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}