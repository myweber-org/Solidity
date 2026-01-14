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
    bool running;

    void populateFileMap() {
        file_timestamps.clear();
        for (const auto& entry : fs::directory_iterator(directory_to_watch)) {
            if (fs::is_regular_file(entry.path())) {
                file_timestamps[entry.path().filename().string()] = fs::last_write_time(entry.path());
            }
        }
    }

public:
    FileSystemWatcher(const std::string& path) : directory_to_watch(path), running(false) {
        if (!fs::exists(directory_to_watch) || !fs::is_directory(directory_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        populateFileMap();
    }

    void startWatching(int interval_seconds = 1) {
        running = true;
        std::cout << "Started watching directory: " << directory_to_watch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            for (const auto& entry : fs::directory_iterator(directory_to_watch)) {
                if (!fs::is_regular_file(entry.path())) {
                    continue;
                }

                auto filename = entry.path().filename().string();
                auto current_write_time = fs::last_write_time(entry.path());

                if (file_timestamps.find(filename) == file_timestamps.end()) {
                    std::cout << "[NEW] File created: " << filename << std::endl;
                    file_timestamps[filename] = current_write_time;
                } else if (file_timestamps[filename] != current_write_time) {
                    std::cout << "[MODIFIED] File changed: " << filename << std::endl;
                    file_timestamps[filename] = current_write_time;
                }
            }

            // Check for deleted files
            auto it = file_timestamps.begin();
            while (it != file_timestamps.end()) {
                auto filepath = directory_to_watch / it->first;
                if (!fs::exists(filepath)) {
                    std::cout << "[DELETED] File removed: " << it->first << std::endl;
                    it = file_timestamps.erase(it);
                } else {
                    ++it;
                }
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
        FileSystemWatcher watcher(".");
        watcher.startWatching(2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}