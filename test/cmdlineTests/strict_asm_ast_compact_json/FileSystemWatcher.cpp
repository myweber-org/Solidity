#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& path) : watch_path(path) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Path does not exist or is not a directory");
        }
        build_file_map();
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

    void build_file_map() {
        file_map.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                file_map[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void check_for_changes() {
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                std::string path_str = entry.path().string();
                auto current_time = fs::last_write_time(entry);

                if (file_map.find(path_str) == file_map.end()) {
                    std::cout << "[NEW] " << path_str << std::endl;
                    file_map[path_str] = current_time;
                } else if (file_map[path_str] != current_time) {
                    std::cout << "[MODIFIED] " << path_str << std::endl;
                    file_map[path_str] = current_time;
                }
            }
        }

        std::vector<std::string> to_remove;
        for (const auto& [path, time] : file_map) {
            if (!fs::exists(path)) {
                std::cout << "[DELETED] " << path << std::endl;
                to_remove.push_back(path);
            }
        }

        for (const auto& path : to_remove) {
            file_map.erase(path);
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