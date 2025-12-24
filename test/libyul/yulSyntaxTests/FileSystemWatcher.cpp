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

    void populate_timestamps() {
        file_timestamps.clear();
        if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                if (fs::is_regular_file(entry.path())) {
                    file_timestamps[entry.path().filename().string()] = fs::last_write_time(entry.path());
                }
            }
        }
    }

public:
    explicit FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        populate_timestamps();
    }

    void start_watching(int interval_seconds = 1) {
        running = true;
        std::cout << "Watching directory: " << path_to_wwatch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
                std::cerr << "Directory does not exist or is not accessible." << std::endl;
                break;
            }

            std::unordered_map<std::string, fs::file_time_type> current_timestamps;

            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                if (fs::is_regular_file(entry.path())) {
                    std::string filename = entry.path().filename().string();
                    current_timestamps[filename] = fs::last_write_time(entry.path());
                }
            }

            for (const auto& [filename, old_time] : file_timestamps) {
                if (current_timestamps.find(filename) == current_timestamps.end()) {
                    std::cout << "File deleted: " << filename << std::endl;
                }
            }

            for (const auto& [filename, new_time] : current_timestamps) {
                auto it = file_timestamps.find(filename);
                if (it == file_timestamps.end()) {
                    std::cout << "File created: " << filename << std::endl;
                } else if (it->second != new_time) {
                    std::cout << "File modified: " << filename << std::endl;
                }
            }

            file_timestamps = std::move(current_timestamps);
        }
    }

    void stop_watching() {
        running = false;
    }
};

int main() {
    FileSystemWatcher watcher(".");
    watcher.start_watching(2);
    return 0;
}