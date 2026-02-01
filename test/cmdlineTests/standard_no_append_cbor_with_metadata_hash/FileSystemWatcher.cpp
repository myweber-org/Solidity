
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class SimpleFileWatcher {
private:
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps;
    bool running;

    void populate_timestamps() {
        file_timestamps.clear();
        if (fs::exists(watch_path) && fs::is_directory(watch_path)) {
            for (const auto& entry : fs::directory_iterator(watch_path)) {
                if (fs::is_regular_file(entry.path())) {
                    file_timestamps[entry.path().string()] = fs::last_write_time(entry.path());
                }
            }
        }
    }

public:
    SimpleFileWatcher(const std::string& path) : watch_path(path), running(false) {
        if (!fs::exists(watch_path)) {
            throw std::runtime_error("Path does not exist: " + path);
        }
        populate_timestamps();
    }

    void start_watching(int interval_seconds = 1) {
        running = true;
        std::cout << "Watching directory: " << watch_path.string() << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            if (!fs::exists(watch_path)) {
                std::cout << "Watched path no longer exists. Stopping." << std::endl;
                stop();
                break;
            }

            std::unordered_map<std::string, fs::file_time_type> current_timestamps;
            for (const auto& entry : fs::directory_iterator(watch_path)) {
                if (fs::is_regular_file(entry.path())) {
                    current_timestamps[entry.path().string()] = fs::last_write_time(entry.path());
                }
            }

            for (const auto& [path, timestamp] : current_timestamps) {
                if (file_timestamps.find(path) == file_timestamps.end()) {
                    std::cout << "File created: " << path << std::endl;
                } else if (file_timestamps[path] != timestamp) {
                    std::cout << "File modified: " << path << std::endl;
                }
            }

            for (const auto& [path, timestamp] : file_timestamps) {
                if (current_timestamps.find(path) == current_timestamps.end()) {
                    std::cout << "File deleted: " << path << std::endl;
                }
            }

            file_timestamps = std::move(current_timestamps);
        }
    }

    void stop() {
        running = false;
    }

    ~SimpleFileWatcher() {
        stop();
    }
};

int main() {
    try {
        SimpleFileWatcher watcher(".");
        watcher.start_watching(2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}