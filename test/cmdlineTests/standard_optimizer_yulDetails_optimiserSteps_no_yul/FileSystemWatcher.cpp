#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const fs::path& path_to_watch) : path_to_watch_(path_to_watch) {
        for (const auto& entry : fs::directory_iterator(path_to_watch_)) {
            paths_[entry.path()] = fs::last_write_time(entry);
        }
    }

    void startMonitoring(int interval_seconds = 1) {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            checkForChanges();
        }
    }

    void stopMonitoring() {
        running_ = false;
    }

private:
    void checkForChanges() {
        auto it = paths_.begin();
        while (it != paths_.end()) {
            if (!fs::exists(it->first)) {
                std::cout << "File deleted: " << it->first << std::endl;
                it = paths_.erase(it);
            } else {
                ++it;
            }
        }

        for (const auto& entry : fs::directory_iterator(path_to_watch_)) {
            auto current_last_write_time = fs::last_write_time(entry);

            if (!paths_.count(entry.path())) {
                paths_[entry.path()] = current_last_write_time;
                std::cout << "File created: " << entry.path() << std::endl;
            } else {
                if (paths_[entry.path()] != current_last_write_time) {
                    paths_[entry.path()] = current_last_write_time;
                    std::cout << "File modified: " << entry.path() << std::endl;
                }
            }
        }
    }

    fs::path path_to_watch_;
    std::unordered_map<fs::path, fs::file_time_type> paths_;
    bool running_ = true;
};

int main() {
    fs::path path_to_watch = "./watch_directory";
    if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
        std::cerr << "Directory does not exist or is not a directory: " << path_to_watch << std::endl;
        return 1;
    }

    FileSystemWatcher watcher(path_to_watch);
    std::cout << "Starting to monitor directory: " << path_to_watch << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;

    watcher.startMonitoring(2);

    return 0;
}#include <iostream>
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
            if (fs::is_regular_file(entry.path())) {
                file_timestamps[entry.path().filename().string()] = fs::last_write_time(entry.path());
            }
        }
    }

public:
    explicit FileSystemWatcher(const std::string& dir_path) : directory_to_watch(dir_path) {
        if (!fs::exists(directory_to_watch) || !fs::is_directory(directory_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        populate_file_map();
    }

    void start_watching(int interval_seconds = 1) {
        running = true;
        std::cout << "Started watching directory: " << directory_to_watch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            for (const auto& entry : fs::directory_iterator(directory_to_watch)) {
                if (!fs::is_regular_file(entry.path())) {
                    continue;
                }

                std::string filename = entry.path().filename().string();
                auto current_write_time = fs::last_write_time(entry.path());

                if (file_timestamps.find(filename) == file_timestamps.end()) {
                    std::cout << "New file detected: " << filename << std::endl;
                    file_timestamps[filename] = current_write_time;
                } else if (file_timestamps[filename] != current_write_time) {
                    std::cout << "File modified: " << filename << std::endl;
                    file_timestamps[filename] = current_write_time;
                }
            }

            // Check for deleted files
            auto it = file_timestamps.begin();
            while (it != file_timestamps.end()) {
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

    void stop_watching() {
        running = false;
        std::cout << "Stopped watching directory." << std::endl;
    }
};

int main() {
    try {
        FileSystemWatcher watcher("./test_directory");
        watcher.start_watching(2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}