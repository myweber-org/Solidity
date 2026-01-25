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

    void populate_file_map() {
        file_timestamps.clear();
        for (const auto& entry : fs::directory_iterator(directory_to_watch)) {
            if (fs::is_regular_file(entry.path())) {
                file_timestamps[entry.path().filename().string()] = fs::last_write_time(entry);
            }
        }
    }

public:
    FileSystemWatcher(const std::string& directory) : directory_to_watch(directory), running(false) {
        if (!fs::exists(directory_to_watch) || !fs::is_directory(directory_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        populate_file_map();
    }

    void start(int interval_seconds = 1) {
        running = true;
        std::cout << "Started watching directory: " << directory_to_watch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            for (const auto& entry : fs::directory_iterator(directory_to_watch)) {
                if (fs::is_regular_file(entry.path())) {
                    std::string filename = entry.path().filename().string();
                    auto current_write_time = fs::last_write_time(entry);

                    if (file_timestamps.find(filename) == file_timestamps.end()) {
                        std::cout << "New file detected: " << filename << std::endl;
                        file_timestamps[filename] = current_write_time;
                    } else if (file_timestamps[filename] != current_write_time) {
                        std::cout << "File modified: " << filename << std::endl;
                        file_timestamps[filename] = current_write_time;
                    }
                }
            }

            for (auto it = file_timestamps.begin(); it != file_timestamps.end(); ) {
                fs::path file_path = directory_to_watch / it->first;
                if (!fs::exists(file_path)) {
                    std::cout << "File deleted: " << it->first << std::endl;
                    it = file_timestamps.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    void stop() {
        running = false;
        std::cout << "Stopped watching directory." << std::endl;
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start(2);

        std::this_thread::sleep_for(std::chrono::seconds(10));
        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}