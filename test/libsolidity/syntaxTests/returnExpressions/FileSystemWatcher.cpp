#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps;
    bool running;

    void populateTimestamps() {
        file_timestamps.clear();
        if (fs::exists(watch_path) && fs::is_directory(watch_path)) {
            for (const auto& entry : fs::directory_iterator(watch_path)) {
                if (fs::is_regular_file(entry.path())) {
                    auto ftime = fs::last_write_time(entry.path());
                    file_timestamps[entry.path().string()] = ftime;
                }
            }
        }
    }

public:
    FileSystemWatcher(const std::string& path) : watch_path(path), running(false) {
        if (!fs::exists(watch_path)) {
            throw std::runtime_error("Path does not exist: " + path);
        }
        populateTimestamps();
    }

    void start(int interval_seconds = 1) {
        running = true;
        std::cout << "Watching directory: " << watch_path.string() << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            if (!fs::exists(watch_path)) {
                std::cout << "Watched path has been removed. Stopping." << std::endl;
                stop();
                break;
            }

            std::unordered_map<std::string, fs::file_time_type> current_timestamps;

            for (const auto& entry : fs::directory_iterator(watch_path)) {
                if (fs::is_regular_file(entry.path())) {
                    auto path_str = entry.path().string();
                    auto ftime = fs::last_write_time(entry.path());
                    current_timestamps[path_str] = ftime;

                    if (file_timestamps.find(path_str) == file_timestamps.end()) {
                        std::cout << "File created: " << path_str << std::endl;
                    } else if (file_timestamps[path_str] != ftime) {
                        std::cout << "File modified: " << path_str << std::endl;
                    }
                }
            }

            for (const auto& old_pair : file_timestamps) {
                if (current_timestamps.find(old_pair.first) == current_timestamps.end()) {
                    std::cout << "File deleted: " << old_pair.first << std::endl;
                }
            }

            file_timestamps.swap(current_timestamps);
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