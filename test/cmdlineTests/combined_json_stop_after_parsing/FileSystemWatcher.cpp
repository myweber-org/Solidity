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

    void populate_timestamps() {
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
        populate_timestamps();
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

            std::vector<std::string> files_to_remove;
            for (const auto& [filename, _] : file_timestamps) {
                if (!fs::exists(directory_to_watch / filename)) {
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