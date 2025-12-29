
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& directory_path)
        : watch_path(directory_path) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        populate_file_map();
    }

    void start_monitoring(int interval_seconds = 1) {
        std::cout << "Starting to monitor: " << watch_path.string() << std::endl;
        while (monitoring_active) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

    void stop_monitoring() {
        monitoring_active = false;
    }

private:
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_map;
    bool monitoring_active{true};

    void populate_file_map() {
        file_map.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.status())) {
                file_map[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void check_for_changes() {
        auto current_files = file_map;
        bool changes_detected = false;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (!fs::is_regular_file(entry.status())) continue;

            std::string file_path = entry.path().string();
            auto current_write_time = fs::last_write_time(entry);

            if (file_map.find(file_path) == file_map.end()) {
                std::cout << "[NEW] File created: " << file_path << std::endl;
                changes_detected = true;
            } else if (file_map[file_path] != current_write_time) {
                std::cout << "[MODIFIED] File changed: " << file_path << std::endl;
                changes_detected = true;
            }
            current_files[file_path] = current_write_time;
        }

        for (const auto& [file_path, _] : file_map) {
            if (current_files.find(file_path) == current_files.end()) {
                std::cout << "[DELETED] File removed: " << file_path << std::endl;
                changes_detected = true;
            }
        }

        if (changes_detected) {
            file_map.swap(current_files);
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start_monitoring(2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}