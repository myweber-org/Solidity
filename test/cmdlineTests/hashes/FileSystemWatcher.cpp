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

    void start(int interval_seconds = 2) {
        running = true;
        std::cout << "Starting watch loop (interval: " << interval_seconds << "s). Press Ctrl+C to stop." << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

    void stop() {
        running = false;
    }

    void check_for_changes() {
        bool changes_detected = false;
        auto current_files = std::unordered_map<std::string, fs::file_time_type>{};

        for (const auto& entry : fs::directory_iterator(directory_to_watch)) {
            if (fs::is_regular_file(entry.path())) {
                std::string filename = entry.path().filename().string();
                auto current_time = fs::last_write_time(entry.path());
                current_files[filename] = current_time;

                if (file_timestamps.find(filename) == file_timestamps.end()) {
                    std::cout << "[NEW] File added: " << filename << std::endl;
                    changes_detected = true;
                } else if (file_timestamps[filename] != current_time) {
                    std::cout << "[MODIFIED] File changed: " << filename << std::endl;
                    changes_detected = true;
                }
            }
        }

        for (const auto& [filename, _] : file_timestamps) {
            if (current_files.find(filename) == current_files.end()) {
                std::cout << "[DELETED] File removed: " << filename << std::endl;
                changes_detected = true;
            }
        }

        if (changes_detected) {
            file_timestamps.swap(current_files);
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}