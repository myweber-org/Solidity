
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path path_to_watch;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps;
    bool running = false;

    bool path_exists(const fs::path& p) {
        return fs::exists(p);
    }

    void populate_file_map() {
        file_timestamps.clear();
        if (path_exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                if (fs::is_regular_file(entry.path())) {
                    file_timestamps[entry.path().string()] = fs::last_write_time(entry.path());
                }
            }
        }
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!path_exists(path_to_watch)) {
            std::cerr << "Path does not exist: " << path_to_watch << std::endl;
        }
        populate_file_map();
    }

    void start_watching(int interval_seconds = 1) {
        running = true;
        std::cout << "Started watching: " << path_to_watch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            if (!path_exists(path_to_watch)) {
                std::cout << "Path no longer exists. Stopping watch." << std::endl;
                stop_watching();
                break;
            }

            std::unordered_map<std::string, fs::file_time_type> current_timestamps;

            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                if (fs::is_regular_file(entry.path())) {
                    std::string file_path = entry.path().string();
                    current_timestamps[file_path] = fs::last_write_time(entry.path());
                }
            }

            for (const auto& [file, old_time] : file_timestamps) {
                if (current_timestamps.find(file) == current_timestamps.end()) {
                    std::cout << "File deleted: " << file << std::endl;
                }
            }

            for (const auto& [file, new_time] : current_timestamps) {
                auto it = file_timestamps.find(file);
                if (it == file_timestamps.end()) {
                    std::cout << "File created: " << file << std::endl;
                } else if (it->second != new_time) {
                    std::cout << "File modified: " << file << std::endl;
                }
            }

            file_timestamps = std::move(current_timestamps);
        }
    }

    void stop_watching() {
        running = false;
        std::cout << "Stopped watching." << std::endl;
    }
};

int main() {
    std::string watch_path = ".";
    FileSystemWatcher watcher(watch_path);
    watcher.start_watching(2);
    return 0;
}