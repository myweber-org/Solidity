#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path directory_path;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps;

    void populate_timestamps() {
        file_timestamps.clear();
        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (fs::is_regular_file(entry.status())) {
                file_timestamps[entry.path().filename().string()] = fs::last_write_time(entry);
            }
        }
    }

public:
    explicit FileSystemWatcher(const std::string& path) : directory_path(path) {
        if (!fs::exists(directory_path) || !fs::is_directory(directory_path)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        populate_timestamps();
        std::cout << "Watching directory: " << directory_path << std::endl;
    }

    void check_for_changes() {
        auto current_timestamps = file_timestamps;
        populate_timestamps();

        for (const auto& [filename, old_time] : current_timestamps) {
            auto it = file_timestamps.find(filename);
            if (it == file_timestamps.end()) {
                std::cout << "File deleted: " << filename << std::endl;
            } else if (it->second != old_time) {
                std::cout << "File modified: " << filename << std::endl;
            }
        }

        for (const auto& [filename, new_time] : file_timestamps) {
            if (current_timestamps.find(filename) == current_timestamps.end()) {
                std::cout << "File created: " << filename << std::endl;
            }
        }
    }

    void start_monitoring(int interval_seconds) {
        std::cout << "Starting monitoring with interval of " << interval_seconds << " seconds." << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            try {
                check_for_changes();
            } catch (const std::exception& e) {
                std::cerr << "Error checking for changes: " << e.what() << std::endl;
            }
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
        watcher.start_monitoring(5);
    } catch (const std::exception& e) {
        std::cerr << "Failed to start watcher: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}