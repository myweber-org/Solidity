
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>
#include <functional>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    using FileChangeCallback = std::function<void(const fs::path&, const std::string&)>;

    SimpleFileWatcher(const fs::path& watch_path, FileChangeCallback callback)
        : watch_path_(watch_path), callback_(callback), running_(false) {
        if (!fs::exists(watch_path_)) {
            throw std::runtime_error("Watch path does not exist");
        }
        scanFiles();
    }

    void start() {
        running_ = true;
        watcher_thread_ = std::thread(&SimpleFileWatcher::watchLoop, this);
    }

    void stop() {
        running_ = false;
        if (watcher_thread_.joinable()) {
            watcher_thread_.join();
        }
    }

    ~SimpleFileWatcher() {
        stop();
    }

private:
    void scanFiles() {
        file_states_.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (entry.is_regular_file()) {
                auto last_write = fs::last_write_time(entry.path());
                file_states_[entry.path()] = last_write;
            }
        }
    }

    void watchLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            checkForChanges();
        }
    }

    void checkForChanges() {
        std::unordered_map<fs::path, fs::file_time_type> current_states;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (entry.is_regular_file()) {
                auto path = entry.path();
                auto last_write = fs::last_write_time(path);
                current_states[path] = last_write;

                auto it = file_states_.find(path);
                if (it == file_states_.end()) {
                    if (callback_) {
                        callback_(path, "created");
                    }
                } else if (it->second != last_write) {
                    if (callback_) {
                        callback_(path, "modified");
                    }
                }
            }
        }

        for (const auto& [path, _] : file_states_) {
            if (current_states.find(path) == current_states.end()) {
                if (callback_) {
                    callback_(path, "deleted");
                }
            }
        }

        file_states_.swap(current_states);
    }

    fs::path watch_path_;
    FileChangeCallback callback_;
    std::unordered_map<fs::path, fs::file_time_type> file_states_;
    std::thread watcher_thread_;
    bool running_;
};

void exampleCallback(const fs::path& path, const std::string& change_type) {
    std::cout << "File: " << path.string() << " - Action: " << change_type << std::endl;
}

int main() {
    try {
        fs::path watch_dir = ".";
        SimpleFileWatcher watcher(watch_dir, exampleCallback);
        
        std::cout << "Watching directory: " << fs::absolute(watch_dir).string() << std::endl;
        std::cout << "Press Enter to stop watching..." << std::endl;
        
        watcher.start();
        std::cin.get();
        watcher.stop();
        
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
private:
    fs::path path_to_watch;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps;
    bool running = false;

    void populate_file_map() {
        file_timestamps.clear();
        if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                if (fs::is_regular_file(entry.path())) {
                    file_timestamps[entry.path().string()] = fs::last_write_time(entry.path());
                }
            }
        }
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        populate_file_map();
    }

    void start(int interval_seconds = 1) {
        running = true;
        std::cout << "Watching directory: " << path_to_watch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
                std::cerr << "Directory does not exist or is not accessible." << std::endl;
                continue;
            }

            std::unordered_map<std::string, fs::file_time_type> current_timestamps;

            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                if (fs::is_regular_file(entry.path())) {
                    std::string file_path = entry.path().string();
                    current_timestamps[file_path] = fs::last_write_time(entry.path());
                }
            }

            for (const auto& [file, timestamp] : current_timestamps) {
                if (file_timestamps.find(file) == file_timestamps.end()) {
                    std::cout << "File created: " << file << std::endl;
                } else if (file_timestamps[file] != timestamp) {
                    std::cout << "File modified: " << file << std::endl;
                }
            }

            for (const auto& [file, timestamp] : file_timestamps) {
                if (current_timestamps.find(file) == current_timestamps.end()) {
                    std::cout << "File deleted: " << file << std::endl;
                }
            }

            file_timestamps = std::move(current_timestamps);
        }
    }

    void stop() {
        running = false;
    }
};

int main() {
    FileSystemWatcher watcher(".");
    watcher.start(2);
    return 0;
}