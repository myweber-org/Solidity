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

    void populateFileMap() {
        file_timestamps.clear();
        for (const auto& entry : fs::directory_iterator(directory_to_watch)) {
            if (fs::is_regular_file(entry.path())) {
                file_timestamps[entry.path().filename().string()] = fs::last_write_time(entry.path());
            }
        }
    }

public:
    FileSystemWatcher(const std::string& directory) : directory_to_watch(directory) {
        if (!fs::exists(directory_to_watch) || !fs::is_directory(directory_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        populateFileMap();
    }

    void startMonitoring(int interval_seconds = 2) {
        std::cout << "Starting to monitor directory: " << directory_to_watch << std::endl;
        std::cout << "Monitoring interval: " << interval_seconds << " seconds." << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            checkForChanges();
        }
    }

    void checkForChanges() {
        bool changes_detected = false;
        auto current_files = file_timestamps;

        for (const auto& entry : fs::directory_iterator(directory_to_watch)) {
            if (fs::is_regular_file(entry.path())) {
                std::string filename = entry.path().filename().string();
                auto current_write_time = fs::last_write_time(entry.path());

                if (file_timestamps.find(filename) == file_timestamps.end()) {
                    std::cout << "New file detected: " << filename << std::endl;
                    changes_detected = true;
                } else if (file_timestamps[filename] != current_write_time) {
                    std::cout << "File modified: " << filename << std::endl;
                    changes_detected = true;
                }
                current_files[filename] = current_write_time;
            }
        }

        for (const auto& [filename, _] : file_timestamps) {
            if (current_files.find(filename) == current_files.end()) {
                std::cout << "File deleted: " << filename << std::endl;
                changes_detected = true;
            }
        }

        if (changes_detected) {
            file_timestamps = std::move(current_files);
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.startMonitoring(3);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}