#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const std::string& path) : watch_path(path) {
        if (fs::exists(watch_path) && fs::is_directory(watch_path)) {
            populate_file_map();
        }
    }

    void start_watching(int interval_seconds = 1) {
        std::cout << "Watching directory: " << watch_path << std::endl;
        std::cout << "Press Ctrl+C to stop." << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    std::string watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_map;

    void populate_file_map() {
        file_map.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_map[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void check_for_changes() {
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                std::string file_path = entry.path().string();
                auto current_write_time = fs::last_write_time(entry);

                if (file_map.find(file_path) == file_map.end()) {
                    std::cout << "[NEW] " << file_path << std::endl;
                    file_map[file_path] = current_write_time;
                } else if (file_map[file_path] != current_write_time) {
                    std::cout << "[MODIFIED] " << file_path << std::endl;
                    file_map[file_path] = current_write_time;
                }
            }
        }

        std::vector<std::string> files_to_remove;
        for (const auto& [file_path, _] : file_map) {
            if (!fs::exists(file_path)) {
                std::cout << "[DELETED] " << file_path << std::endl;
                files_to_remove.push_back(file_path);
            }
        }

        for (const auto& file_path : files_to_remove) {
            file_map.erase(file_path);
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
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