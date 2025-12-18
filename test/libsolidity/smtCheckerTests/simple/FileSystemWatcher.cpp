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

    void start_watching(int interval_seconds = 2) {
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
                file_map[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void check_for_changes() {
        auto it = file_map.begin();
        while (it != file_map.end()) {
            if (!fs::exists(it->first)) {
                std::cout << "File deleted: " << it->first << std::endl;
                it = file_map.erase(it);
            } else {
                ++it;
            }
        }

        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.path())) {
                std::string file_path = entry.path().string();
                auto current_write_time = fs::last_write_time(entry);

                if (file_map.find(file_path) == file_map.end()) {
                    std::cout << "New file created: " << file_path << std::endl;
                    file_map[file_path] = current_write_time;
                } else if (file_map[file_path] != current_write_time) {
                    std::cout << "File modified: " << file_path << std::endl;
                    file_map[file_path] = current_write_time;
                }
            }
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