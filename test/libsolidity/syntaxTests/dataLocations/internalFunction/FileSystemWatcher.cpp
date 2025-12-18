
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>
#include <ctime>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path directory_path;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps;
    bool running;

    void initializeTimestamps() {
        file_timestamps.clear();
        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (entry.is_regular_file()) {
                file_timestamps[entry.path().filename().string()] = fs::last_write_time(entry);
            }
        }
    }

public:
    FileSystemWatcher(const std::string& path) : directory_path(path), running(false) {
        if (!fs::exists(directory_path) || !fs::is_directory(directory_path)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        initializeTimestamps();
    }

    void startWatching(int interval_seconds = 1) {
        running = true;
        std::cout << "Watching directory: " << directory_path << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            checkForChanges();
        }
    }

    void stopWatching() {
        running = false;
    }

    void checkForChanges() {
        std::vector<std::string> current_files;
        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (entry.is_regular_file()) {
                current_files.push_back(entry.path().filename().string());
            }
        }

        for (const auto& file : current_files) {
            fs::path file_path = directory_path / file;
            auto current_time = fs::last_write_time(file_path);

            if (file_timestamps.find(file) == file_timestamps.end()) {
                std::cout << "[NEW] File created: " << file << std::endl;
                file_timestamps[file] = current_time;
            } else if (file_timestamps[file] != current_time) {
                std::cout << "[MODIFIED] File changed: " << file << std::endl;
                file_timestamps[file] = current_time;
            }
        }

        std::vector<std::string> files_to_remove;
        for (const auto& [file, timestamp] : file_timestamps) {
            if (std::find(current_files.begin(), current_files.end(), file) == current_files.end()) {
                std::cout << "[DELETED] File removed: " << file << std::endl;
                files_to_remove.push_back(file);
            }
        }

        for (const auto& file : files_to_remove) {
            file_timestamps.erase(file);
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.startWatching(2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}