#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path directory_to_watch;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps;
    bool running = false;

    void populate_file_map() {
        file_timestamps.clear();
        for (const auto& entry : fs::directory_iterator(directory_to_watch)) {
            if (fs::is_regular_file(entry.path())) {
                file_timestamps[entry.path().filename().string()] = fs::last_write_time(entry.path());
            }
        }
    }

public:
    explicit FileSystemWatcher(const std::string& dir_path) : directory_to_watch(dir_path) {
        if (!fs::exists(directory_to_watch) || !fs::is_directory(directory_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        populate_file_map();
        std::cout << "Watching directory: " << fs::absolute(directory_to_watch) << std::endl;
    }

    void start(int interval_seconds = 1) {
        running = true;
        std::cout << "Starting watch with interval " << interval_seconds << " second(s)." << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            auto current_timestamps = file_timestamps;
            bool changes_detected = false;

            for (const auto& entry : fs::directory_iterator(directory_to_watch)) {
                if (!fs::is_regular_file(entry.path())) {
                    continue;
                }

                std::string filename = entry.path().filename().string();
                auto current_write_time = fs::last_write_time(entry.path());

                auto it = file_timestamps.find(filename);
                if (it == file_timestamps.end()) {
                    std::cout << "[NEW] File created: " << filename << std::endl;
                    changes_detected = true;
                } else if (it->second != current_write_time) {
                    std::cout << "[MODIFIED] File changed: " << filename << std::endl;
                    changes_detected = true;
                }
                current_timestamps[filename] = current_write_time;
            }

            for (const auto& [filename, _] : file_timestamps) {
                if (current_timestamps.find(filename) == current_timestamps.end()) {
                    std::cout << "[DELETED] File removed: " << filename << std::endl;
                    changes_detected = true;
                }
            }

            if (changes_detected) {
                file_timestamps = std::move(current_timestamps);
            }
        }
    }

    void stop() {
        running = false;
        std::cout << "Stopping file system watcher." << std::endl;
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start(2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}