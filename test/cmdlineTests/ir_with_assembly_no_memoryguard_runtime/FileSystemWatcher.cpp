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
            refresh_file_map();
        }
    }

    void start_watching(int interval_seconds = 2) {
        std::cout << "Watching directory: " << watch_path << std::endl;
        std::cout << "Polling interval: " << interval_seconds << " seconds" << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    std::string watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_map;

    void refresh_file_map() {
        file_map.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_map[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void check_for_changes() {
        auto current_time = std::chrono::system_clock::now();
        std::time_t current_time_t = std::chrono::system_clock::to_time_t(current_time);
        std::cout << "\n[" << std::ctime(&current_time_t) << "] Checking for changes...";

        bool changes_detected = false;
        std::unordered_map<std::string, fs::file_time_type> current_file_map;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                std::string file_path = entry.path().string();
                current_file_map[file_path] = fs::last_write_time(entry.path());

                if (file_map.find(file_path) == file_map.end()) {
                    std::cout << "  [+] New file: " << file_path << std::endl;
                    changes_detected = true;
                } else if (file_map[file_path] != current_file_map[file_path]) {
                    std::cout << "  [*] Modified: " << file_path << std::endl;
                    changes_detected = true;
                }
            }
        }

        for (const auto& [file_path, _] : file_map) {
            if (current_file_map.find(file_path) == current_file_map.end()) {
                std::cout << "  [-] Deleted: " << file_path << std::endl;
                changes_detected = true;
            }
        }

        if (!changes_detected) {
            std::cout << "  No changes detected." << std::endl;
        }

        file_map = std::move(current_file_map);
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