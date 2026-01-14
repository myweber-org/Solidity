#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path path_to_watch;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps;
    bool running = false;

    bool contains(const std::string &key) {
        return file_timestamps.find(key) != file_timestamps.end();
    }

public:
    FileSystemWatcher(const std::string &path) : path_to_watch(path) {
        for (auto &file : fs::recursive_directory_iterator(path_to_watch)) {
            if (fs::is_regular_file(file.path())) {
                file_timestamps[file.path().string()] = fs::last_write_time(file.path());
            }
        }
    }

    void start(int interval_seconds = 1) {
        running = true;
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

    void stop() {
        running = false;
    }

    void check_for_changes() {
        auto it = file_timestamps.begin();
        while (it != file_timestamps.end()) {
            if (!fs::exists(it->first)) {
                std::cout << "File deleted: " << it->first << std::endl;
                it = file_timestamps.erase(it);
            } else {
                ++it;
            }
        }

        for (auto &file : fs::recursive_directory_iterator(path_to_watch)) {
            if (fs::is_regular_file(file.path())) {
                auto current_file_last_write_time = fs::last_write_time(file.path());
                std::string file_path_str = file.path().string();

                if (!contains(file_path_str)) {
                    file_timestamps[file_path_str] = current_file_last_write_time;
                    std::cout << "File created: " << file_path_str << std::endl;
                } else {
                    auto &saved_last_write_time = file_timestamps[file_path_str];
                    if (saved_last_write_time != current_file_last_write_time) {
                        saved_last_write_time = current_file_last_write_time;
                        std::cout << "File modified: " << file_path_str << std::endl;
                    }
                }
            }
        }
    }
};

int main() {
    std::string path = ".";
    FileSystemWatcher watcher(path);
    std::cout << "Watching directory: " << fs::absolute(path) << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;
    watcher.start(2);
    return 0;
}