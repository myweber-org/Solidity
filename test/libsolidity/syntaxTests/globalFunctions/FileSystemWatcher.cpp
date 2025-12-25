#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class SimpleFileWatcher {
private:
    fs::path directory_path;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps;
    bool running;

    void updateTimestamps() {
        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                file_timestamps[filename] = fs::last_write_time(entry);
            }
        }
    }

public:
    SimpleFileWatcher(const std::string& path) : directory_path(path), running(false) {
        if (!fs::exists(directory_path) || !fs::is_directory(directory_path)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        updateTimestamps();
    }

    void startWatching(int interval_seconds = 1) {
        running = true;
        std::cout << "Started watching directory: " << directory_path << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            for (const auto& entry : fs::directory_iterator(directory_path)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    auto current_time = fs::last_write_time(entry);

                    if (file_timestamps.find(filename) == file_timestamps.end()) {
                        std::cout << "New file detected: " << filename << std::endl;
                        file_timestamps[filename] = current_time;
                    } else if (file_timestamps[filename] != current_time) {
                        std::cout << "File modified: " << filename << std::endl;
                        file_timestamps[filename] = current_time;
                    }
                }
            }

            std::vector<std::string> files_to_remove;
            for (const auto& [filename, _] : file_timestamps) {
                if (!fs::exists(directory_path / filename)) {
                    std::cout << "File deleted: " << filename << std::endl;
                    files_to_remove.push_back(filename);
                }
            }
            for (const auto& filename : files_to_remove) {
                file_timestamps.erase(filename);
            }
        }
    }

    void stopWatching() {
        running = false;
        std::cout << "Stopped watching directory." << std::endl;
    }
};

int main() {
    try {
        SimpleFileWatcher watcher(".");
        watcher.startWatching(2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}