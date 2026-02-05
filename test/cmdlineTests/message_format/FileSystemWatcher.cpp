#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileWatcher {
public:
    FileWatcher(const fs::path& path) : watch_path(path) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Path does not exist or is not a directory");
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
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_map[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void check_for_changes() {
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (!fs::is_regular_file(entry.path())) continue;

            auto current_path = entry.path().string();
            auto current_time = fs::last_write_time(entry.path());

            if (file_map.find(current_path) == file_map.end()) {
                std::cout << "[NEW] " << current_path << std::endl;
                file_map[current_path] = current_time;
            } else if (file_map[current_path] != current_time) {
                std::cout << "[MODIFIED] " << current_path << std::endl;
                file_map[current_path] = current_time;
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
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        FileWatcher watcher(argv[1]);
        watcher.start_watching();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}