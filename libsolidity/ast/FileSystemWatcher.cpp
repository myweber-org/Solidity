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
            if (fs::is_regular_file(entry.status())) {
                file_timestamps[entry.path().filename().string()] = fs::last_write_time(entry);
            }
        }
    }

public:
    FileSystemWatcher(const std::string& dir_path) : directory_to_watch(dir_path) {
        if (!fs::exists(directory_to_watch) || !fs::is_directory(directory_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        populate_file_map();
        std::cout << "Watching directory: " << fs::absolute(directory_to_watch) << std::endl;
    }

    void start(int interval_seconds = 1) {
        running = true;
        std::cout << "Starting watch with interval " << interval_seconds << " seconds." << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

    void stop() {
        running = false;
        std::cout << "Stopping file system watcher." << std::endl;
    }

    void check_for_changes() {
        auto it = file_timestamps.begin();
        while (it != file_timestamps.end()) {
            fs::path file_path = directory_to_watch / it->first;
            if (!fs::exists(file_path)) {
                std::cout << "File removed: " << it->first << std::endl;
                it = file_timestamps.erase(it);
            } else {
                ++it;
            }
        }

        for (const auto& entry : fs::directory_iterator(directory_to_watch)) {
            if (fs::is_regular_file(entry.status())) {
                std::string filename = entry.path().filename().string();
                auto current_time = fs::last_write_time(entry);

                if (file_timestamps.find(filename) == file_timestamps.end()) {
                    std::cout << "New file added: " << filename << std::endl;
                    file_timestamps[filename] = current_time;
                } else {
                    if (file_timestamps[filename] != current_time) {
                        std::cout << "File modified: " << filename << std::endl;
                        file_timestamps[filename] = current_time;
                    }
                }
            }
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        std::thread watch_thread([&watcher]() {
            watcher.start(2);
        });

        std::cout << "Press Enter to stop watching..." << std::endl;
        std::cin.get();

        watcher.stop();
        watch_thread.join();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}