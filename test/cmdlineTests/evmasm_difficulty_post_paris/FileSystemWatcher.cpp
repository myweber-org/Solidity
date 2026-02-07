#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& directory) : watch_path(directory) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        populate_file_map();
    }

    void start_watching(int interval_seconds = 1) {
        std::cout << "Watching directory: " << watch_path << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_map;

    void populate_file_map() {
        file_map.clear();
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.status())) {
                file_map[entry.path().filename().string()] = fs::last_write_time(entry);
            }
        }
    }

    void check_for_changes() {
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (!fs::is_regular_file(entry.status())) continue;

            std::string filename = entry.path().filename().string();
            auto current_write_time = fs::last_write_time(entry);

            if (file_map.find(filename) == file_map.end()) {
                std::cout << "New file detected: " << filename << std::endl;
                file_map[filename] = current_write_time;
            } else if (file_map[filename] != current_write_time) {
                std::cout << "File modified: " << filename << std::endl;
                file_map[filename] = current_write_time;
            }
        }

        std::vector<std::string> files_to_remove;
        for (const auto& [filename, _] : file_map) {
            if (!fs::exists(watch_path / filename)) {
                std::cout << "File deleted: " << filename << std::endl;
                files_to_remove.push_back(filename);
            }
        }

        for (const auto& filename : files_to_remove) {
            file_map.erase(filename);
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
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