#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const fs::path& path_to_watch) : path_to_watch_(path_to_watch) {
        if (!fs::exists(path_to_watch_) || !fs::is_directory(path_to_watch_)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        populate_file_map();
    }

    void start_monitoring(int interval_seconds = 1) {
        std::cout << "Starting to monitor: " << path_to_watch_ << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path path_to_watch_;
    std::unordered_map<std::string, fs::file_time_type> file_map_;

    void populate_file_map() {
        file_map_.clear();
        for (const auto& entry : fs::recursive_directory_iterator(path_to_watch_)) {
            if (fs::is_regular_file(entry.status())) {
                file_map_[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void check_for_changes() {
        auto it = file_map_.begin();
        while (it != file_map_.end()) {
            if (!fs::exists(it->first)) {
                std::cout << "File deleted: " << it->first << std::endl;
                it = file_map_.erase(it);
            } else {
                ++it;
            }
        }

        for (const auto& entry : fs::recursive_directory_iterator(path_to_watch_)) {
            if (fs::is_regular_file(entry.status())) {
                std::string file_path = entry.path().string();
                auto current_write_time = fs::last_write_time(entry);

                if (file_map_.find(file_path) == file_map_.end()) {
                    std::cout << "File created: " << file_path << std::endl;
                    file_map_[file_path] = current_write_time;
                } else if (file_map_[file_path] != current_write_time) {
                    std::cout << "File modified: " << file_path << std::endl;
                    file_map_[file_path] = current_write_time;
                }
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start_monitoring();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}