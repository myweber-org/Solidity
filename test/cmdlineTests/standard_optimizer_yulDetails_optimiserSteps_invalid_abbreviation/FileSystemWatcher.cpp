
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    SimpleFileWatcher(const fs::path& directory) : watch_path(directory) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        populate_file_map();
    }

    void start_watching(int interval_seconds = 2) {
        std::cout << "Watching directory: " << watch_path.string() << std::endl;
        std::cout << "Polling interval: " << interval_seconds << " seconds" << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps;

    void populate_file_map() {
        file_timestamps.clear();
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.status())) {
                file_timestamps[entry.path().filename().string()] = fs::last_write_time(entry);
            }
        }
    }

    void check_for_changes() {
        bool changes_detected = false;
        auto current_files = file_timestamps;

        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (!fs::is_regular_file(entry.status())) continue;

            std::string filename = entry.path().filename().string();
            auto current_time = fs::last_write_time(entry);

            if (file_timestamps.find(filename) == file_timestamps.end()) {
                std::cout << "[NEW] File created: " << filename << std::endl;
                changes_detected = true;
            } else if (file_timestamps[filename] != current_time) {
                std::cout << "[MODIFIED] File changed: " << filename << std::endl;
                changes_detected = true;
            }
            current_files[filename] = current_time;
        }

        for (const auto& [filename, _] : file_timestamps) {
            if (current_files.find(filename) == current_files.end()) {
                std::cout << "[DELETED] File removed: " << filename << std::endl;
                changes_detected = true;
            }
        }

        if (changes_detected) {
            file_timestamps = std::move(current_files);
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
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