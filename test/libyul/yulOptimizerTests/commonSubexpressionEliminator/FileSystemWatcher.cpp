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

    void populate_file_map() {
        file_timestamps.clear();
        if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                if (fs::is_regular_file(entry.path())) {
                    file_timestamps[entry.path().string()] = fs::last_write_time(entry.path());
                }
            }
        }
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        populate_file_map();
    }

    void start(int interval_seconds = 1) {
        running = true;
        std::cout << "Watching directory: " << path_to_watch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
                std::cerr << "Directory does not exist or is not accessible." << std::endl;
                continue;
            }

            std::unordered_map<std::string, fs::file_time_type> current_timestamps;

            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                if (fs::is_regular_file(entry.path())) {
                    std::string file_path = entry.path().string();
                    current_timestamps[file_path] = fs::last_write_time(entry.path());
                }
            }

            for (const auto& [file, timestamp] : current_timestamps) {
                if (file_timestamps.find(file) == file_timestamps.end()) {
                    std::cout << "File created: " << file << std::endl;
                } else if (file_timestamps[file] != timestamp) {
                    std::cout << "File modified: " << file << std::endl;
                }
            }

            for (const auto& [file, timestamp] : file_timestamps) {
                if (current_timestamps.find(file) == current_timestamps.end()) {
                    std::cout << "File deleted: " << file << std::endl;
                }
            }

            file_timestamps = std::move(current_timestamps);
        }
    }

    void stop() {
        running = false;
    }
};

int main() {
    FileSystemWatcher watcher(".");
    watcher.start(2);
    return 0;
}