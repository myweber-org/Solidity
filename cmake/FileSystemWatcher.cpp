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
    bool running;

    void populateTimestamps() {
        file_timestamps.clear();
        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_timestamps[entry.path().filename().string()] = fs::last_write_time(entry.path());
            }
        }
    }

public:
    FileSystemWatcher(const std::string& path) : directory_path(path), running(false) {
        if (!fs::exists(directory_path) || !fs::is_directory(directory_path)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        populateTimestamps();
    }

    void start(int interval_seconds = 1) {
        running = true;
        std::cout << "Watching directory: " << directory_path << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            checkForChanges();
        }
    }

    void stop() {
        running = false;
    }

    void checkForChanges() {
        bool changes_detected = false;
        auto current_files = std::unordered_map<std::string, fs::file_time_type>{};

        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (fs::is_regular_file(entry.path())) {
                std::string filename = entry.path().filename().string();
                auto current_time = fs::last_write_time(entry.path());
                current_files[filename] = current_time;

                if (file_timestamps.find(filename) == file_timestamps.end()) {
                    std::cout << "File added: " << filename << std::endl;
                    changes_detected = true;
                } else if (file_timestamps[filename] != current_time) {
                    std::cout << "File modified: " << filename << std::endl;
                    changes_detected = true;
                }
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
        watcher.start(2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}