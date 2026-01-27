#include <iostream>
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

    void start_monitoring(int interval_seconds = 1) {
        std::cout << "Starting to monitor: " << watch_path.string() << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_map;

    void populate_file_map() {
        file_map.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_map[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void check_for_changes() {
        auto current_files = file_map;
        bool changes_detected = false;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                std::string file_path = entry.path().string();
                auto current_write_time = fs::last_write_time(entry.path());

                if (file_map.find(file_path) == file_map.end()) {
                    std::cout << "[NEW] " << file_path << std::endl;
                    changes_detected = true;
                } else if (file_map[file_path] != current_write_time) {
                    std::cout << "[MODIFIED] " << file_path << std::endl;
                    changes_detected = true;
                }
                current_files[file_path] = current_write_time;
            }
        }

        for (const auto& [file_path, _] : file_map) {
            if (current_files.find(file_path) == current_files.end()) {
                std::cout << "[DELETED] " << file_path << std::endl;
                changes_detected = true;
            }
        }

        if (changes_detected) {
            file_map.swap(current_files);
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start_monitoring(2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>
#include <functional>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher(const fs::path& watch_path, Callback callback)
        : watch_path_(watch_path), callback_(callback), running_(false) {
        if (!fs::exists(watch_path_) || !fs::is_directory(watch_path_)) {
            throw std::runtime_error("Watch path does not exist or is not a directory");
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

    void start() {
        running_ = true;
        snapshot_ = take_snapshot();
        watcher_thread_ = std::thread(&FileSystemWatcher::watch_loop, this);
    }

    void stop() {
        running_ = false;
        if (watcher_thread_.joinable()) {
            watcher_thread_.join();
        }
    }

private:
    struct FileInfo {
        fs::file_time_type last_write;
        uintmax_t size;
        bool operator==(const FileInfo& other) const {
            return last_write == other.last_write && size == other.size;
        }
    };

    using Snapshot = std::unordered_map<fs::path, FileInfo>;

    Snapshot take_snapshot() {
        Snapshot snapshot;
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (fs::is_regular_file(entry.status())) {
                snapshot[entry.path()] = {
                    fs::last_write_time(entry),
                    fs::file_size(entry)
                };
            }
        }
        return snapshot;
    }

    void watch_loop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            auto current_snapshot = take_snapshot();
            
            // Check for new or modified files
            for (const auto& [path, current_info] : current_snapshot) {
                auto old_it = snapshot_.find(path);
                if (old_it == snapshot_.end()) {
                    callback_(path, "created");
                } else if (!(old_it->second == current_info)) {
                    callback_(path, "modified");
                }
            }
            
            // Check for deleted files
            for (const auto& [path, old_info] : snapshot_) {
                if (current_snapshot.find(path) == current_snapshot.end()) {
                    callback_(path, "deleted");
                }
            }
            
            snapshot_ = std::move(current_snapshot);
        }
    }

    fs::path watch_path_;
    Callback callback_;
    std::atomic<bool> running_;
    std::thread watcher_thread_;
    Snapshot snapshot_;
};

// Example usage
int main() {
    try {
        FileSystemWatcher watcher(
            fs::current_path(),
            [](const fs::path& path, const std::string& action) {
                std::cout << "File " << path.filename() << " was " << action << std::endl;
            }
        );
        
        std::cout << "Watching directory: " << fs::current_path() << std::endl;
        std::cout << "Press Enter to stop watching..." << std::endl;
        
        watcher.start();
        std::cin.get();
        watcher.stop();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}