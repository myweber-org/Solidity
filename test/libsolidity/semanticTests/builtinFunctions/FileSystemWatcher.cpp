#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>
#include <functional>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using FileTimeMap = std::unordered_map<std::string, fs::file_time_type>;
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher(const fs::path& directory, Callback callback)
        : watch_directory(directory), change_callback(callback) {
        if (!fs::exists(watch_directory) || !fs::is_directory(watch_directory)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        initialize_file_times();
    }

    void start_watching(int interval_seconds = 1) {
        std::cout << "Starting file system watcher for: " << watch_directory << std::endl;
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

    void stop_watching() {
        running = false;
    }

private:
    fs::path watch_directory;
    Callback change_callback;
    FileTimeMap last_file_times;
    bool running = true;

    void initialize_file_times() {
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.status())) {
                last_file_times[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void check_for_changes() {
        FileTimeMap current_file_times;

        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.status())) {
                std::string file_path = entry.path().string();
                auto current_time = fs::last_write_time(entry);
                current_file_times[file_path] = current_time;

                auto it = last_file_times.find(file_path);
                if (it == last_file_times.end()) {
                    if (change_callback) {
                        change_callback(entry.path(), "CREATED");
                    }
                } else if (it->second != current_time) {
                    if (change_callback) {
                        change_callback(entry.path(), "MODIFIED");
                    }
                }
            }
        }

        for (const auto& [file_path, _] : last_file_times) {
            if (current_file_times.find(file_path) == current_file_times.end()) {
                if (change_callback) {
                    change_callback(fs::path(file_path), "DELETED");
                }
            }
        }

        last_file_times = std::move(current_file_times);
    }
};

void example_usage() {
    fs::path target_dir = "./watch_folder";
    
    auto callback = [](const fs::path& file_path, const std::string& change_type) {
        std::cout << "File: " << file_path.filename() 
                  << " | Change: " << change_type 
                  << " | Full path: " << file_path << std::endl;
    };

    try {
        FileSystemWatcher watcher(target_dir, callback);
        std::thread watch_thread([&watcher]() {
            watcher.start_watching(2);
        });

        std::cout << "Watcher running. Press Enter to stop..." << std::endl;
        std::cin.get();
        
        watcher.stop_watching();
        watch_thread.join();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}