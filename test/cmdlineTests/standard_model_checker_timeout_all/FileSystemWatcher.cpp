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

    void populate_timestamps() {
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
            throw std::runtime_error("Provided path is not a valid directory.");
        }
        populate_timestamps();
    }

    void start_watching(int interval_seconds = 1) {
        running = true;
        std::cout << "Watching directory: " << directory_path << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            for (const auto& entry : fs::directory_iterator(directory_path)) {
                if (fs::is_regular_file(entry.status())) {
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

    void stop_watching() {
        running = false;
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start_watching(2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}