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
}#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& directory) : watch_path(directory) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        populate_file_map();
    }

    void start_monitoring(int interval_seconds = 2) {
        std::cout << "Starting to monitor: " << watch_path.string() << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps;

    void populate_file_map() {
        file_timestamps.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_timestamps[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
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

        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                std::string file_path = entry.path().string();
                auto current_write_time = fs::last_write_time(entry.path());

                if (file_timestamps.find(file_path) == file_timestamps.end()) {
                    std::cout << "File created: " << file_path << std::endl;
                    file_timestamps[file_path] = current_write_time;
                } else if (file_timestamps[file_path] != current_write_time) {
                    std::cout << "File modified: " << file_path << std::endl;
                    file_timestamps[file_path] = current_write_time;
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