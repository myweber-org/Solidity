
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    SimpleFileWatcher(const std::string& path) : watch_path(path) {
        if (fs::exists(path) && fs::is_directory(path)) {
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
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_map[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void check_for_changes() {
        auto current_files = file_map;
        bool changes_detected = false;

        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (!fs::is_regular_file(entry.path())) {
                continue;
            }

            std::string file_path = entry.path().string();
            auto current_time = fs::last_write_time(entry.path());

            if (file_map.find(file_path) == file_map.end()) {
                std::cout << "[NEW] " << file_path << std::endl;
                changes_detected = true;
            } else if (file_map[file_path] != current_time) {
                std::cout << "[MODIFIED] " << file_path << std::endl;
                changes_detected = true;
            }
            current_files[file_path] = current_time;
        }

        for (const auto& [file_path, _] : file_map) {
            if (!fs::exists(file_path)) {
                std::cout << "[DELETED] " << file_path << std::endl;
                changes_detected = true;
            }
        }

        if (changes_detected) {
            file_map = std::move(current_files);
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        SimpleFileWatcher watcher(argv[1]);
        watcher.start_watching();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}