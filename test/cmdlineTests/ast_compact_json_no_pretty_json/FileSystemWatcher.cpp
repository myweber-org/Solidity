
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& directory) 
        : watch_path(directory), running(false) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Invalid directory path provided");
        }
        populate_file_map();
    }

    void start_watching(int interval_seconds = 1) {
        running = true;
        std::cout << "Started watching: " << watch_path.string() << std::endl;
        
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

    void stop_watching() {
        running = false;
    }

private:
    fs::path watch_path;
    bool running;
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
        auto current_files = std::unordered_map<std::string, fs::file_time_type>{};
        
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                std::string file_path = entry.path().string();
                auto current_time = fs::last_write_time(entry.path());
                current_files[file_path] = current_time;

                if (file_timestamps.find(file_path) == file_timestamps.end()) {
                    std::cout << "[NEW] " << file_path << std::endl;
                } else if (file_timestamps[file_path] != current_time) {
                    std::cout << "[MODIFIED] " << file_path << std::endl;
                }
            }
        }

        for (const auto& [old_file, _] : file_timestamps) {
            if (current_files.find(old_file) == current_files.end()) {
                std::cout << "[DELETED] " << old_file << std::endl;
            }
        }

        file_timestamps = std::move(current_files);
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        std::thread watch_thread([&watcher]() {
            watcher.start_watching(2);
        });

        std::cout << "Press Enter to stop watching..." << std::endl;
        std::cin.get();
        
        watcher.stop_watching();
        watch_thread.join();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}