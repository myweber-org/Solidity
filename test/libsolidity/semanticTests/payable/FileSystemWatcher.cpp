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
            throw std::runtime_error("Invalid directory path provided.");
        }
        populate_file_map();
    }

    void start_watching(int interval_seconds = 1) {
        std::cout << "Watching directory: " << watch_directory << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path watch_directory;
    std::unordered_map<std::string, fs::file_time_type> file_map;

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
            if (fs::is_regular_file(entry.path())) {
                std::string path_str = entry.path().string();
                auto current_write_time = fs::last_write_time(entry.path());

                if (file_map.find(path_str) == file_map.end()) {
                    std::cout << "[NEW] " << path_str << std::endl;
                    changes_detected = true;
                } else if (file_map[path_str] != current_write_time) {
                    std::cout << "[MODIFIED] " << path_str << std::endl;
                    changes_detected = true;
                }
                current_files[path_str] = current_write_time;
            }
        }

        for (const auto& [path, time] : file_map) {
            if (current_files.find(path) == current_files.end()) {
                std::cout << "[DELETED] " << path << std::endl;
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
        watcher.start_watching();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}