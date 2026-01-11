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
            if (fs::is_regular_file(entry.status())) {
                file_timestamps[entry.path().filename().string()] = fs::last_write_time(entry);
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

            bool changed = false;
            auto current_files = file_timestamps;

            for (const auto& entry : fs::directory_iterator(directory_path)) {
                if (fs::is_regular_file(entry.status())) {
                    std::string filename = entry.path().filename().string();
                    auto current_time = fs::last_write_time(entry);

                    if (file_timestamps.find(filename) == file_timestamps.end()) {
                        std::cout << "New file detected: " << filename << std::endl;
                        changed = true;
                    } else if (file_timestamps[filename] != current_time) {
                        std::cout << "File modified: " << filename << std::endl;
                        changed = true;
                    }
                    current_files[filename] = current_time;
                }
            }

            for (auto it = file_timestamps.begin(); it != file_timestamps.end(); ) {
                if (current_files.find(it->first) == current_files.end()) {
                    std::cout << "File deleted: " << it->first << std::endl;
                    it = file_timestamps.erase(it);
                    changed = true;
                } else {
                    ++it;
                }
            }

            if (changed) {
                file_timestamps = current_files;
            }
        }
    }

    void stop() {
        running = false;
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