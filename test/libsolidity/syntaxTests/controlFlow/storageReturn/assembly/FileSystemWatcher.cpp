
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
    using FileChangeCallback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher(const fs::path& directory_path) 
        : watch_path(directory_path), running(false) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Invalid directory path provided");
        }
    }

    void start(FileChangeCallback callback) {
        if (running) return;
        
        running = true;
        snapshot_files();
        
        watcher_thread = std::thread([this, callback]() {
            while (running) {
                std::this_thread::sleep_for(poll_interval);
                detect_changes(callback);
            }
        });
    }

    void stop() {
        running = false;
        if (watcher_thread.joinable()) {
            watcher_thread.join();
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

private:
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_snapshot;
    std::thread watcher_thread;
    bool running;
    const std::chrono::milliseconds poll_interval{1000};

    void snapshot_files() {
        file_snapshot.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                std::string file_key = entry.path().string();
                file_snapshot[file_key] = fs::last_write_time(entry);
            }
        }
    }

    void detect_changes(FileChangeCallback callback) {
        std::unordered_map<std::string, fs::file_time_type> current_state;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                std::string file_key = entry.path().string();
                current_state[file_key] = fs::last_write_time(entry);
            }
        }

        for (const auto& [file_path, current_time] : current_state) {
            auto it = file_snapshot.find(file_path);
            if (it == file_snapshot.end()) {
                callback(fs::path(file_path), "CREATED");
            } else if (it->second != current_time) {
                callback(fs::path(file_path), "MODIFIED");
            }
        }

        for (const auto& [file_path, _] : file_snapshot) {
            if (current_state.find(file_path) == current_state.end()) {
                callback(fs::path(file_path), "DELETED");
            }
        }

        file_snapshot = std::move(current_state);
    }
};

void example_usage() {
    try {
        FileSystemWatcher watcher("./test_directory");
        
        auto callback = [](const fs::path& file_path, const std::string& change_type) {
            std::cout << "File: " << file_path.filename() 
                      << " | Change: " << change_type 
                      << " | Full path: " << file_path << std::endl;
        };

        std::cout << "Starting file system watcher..." << std::endl;
        watcher.start(callback);

        std::this_thread::sleep_for(std::chrono::seconds(30));
        
        watcher.stop();
        std::cout << "File system watcher stopped." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}