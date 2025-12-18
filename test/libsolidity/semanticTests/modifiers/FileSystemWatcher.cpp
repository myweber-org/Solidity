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
        if (!fs::exists(watch_directory) || !fs::is_directory(watch_directory)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        populate_file_map();
    }

    void start_monitoring(int interval_seconds = 1) {
        std::cout << "Starting to monitor: " << watch_directory.string() << std::endl;
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
    bool monitoring_active{true};

    void populate_file_map() {
        file_map.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.path())) {
                file_map[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void check_for_changes() {
        auto current_files = file_map;
        bool changes_detected = false;

        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (!fs::is_regular_file(entry.path())) continue;

            std::string file_path = entry.path().string();
            auto current_write_time = fs::last_write_time(entry.path());

            if (current_files.find(file_path) == current_files.end()) {
                std::cout << "[NEW] File created: " << file_path << std::endl;
                changes_detected = true;
            } else if (current_files[file_path] != current_write_time) {
                std::cout << "[MODIFIED] File changed: " << file_path << std::endl;
                changes_detected = true;
                current_files.erase(file_path);
            } else {
                current_files.erase(file_path);
            }
        }

        for (const auto& [deleted_file, _] : current_files) {
            std::cout << "[DELETED] File removed: " << deleted_file << std::endl;
            changes_detected = true;
        }

        if (changes_detected) {
            populate_file_map();
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