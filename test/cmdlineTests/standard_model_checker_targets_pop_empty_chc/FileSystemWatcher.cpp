#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path path_to_watch;
    std::unordered_set<std::string> current_files;
    bool running = false;

    void populate_file_set() {
        current_files.clear();
        for (const auto& entry : fs::directory_iterator(path_to_watch)) {
            current_files.insert(entry.path().filename().string());
        }
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        populate_file_set();
    }

    void start_watching(int interval_seconds = 1) {
        running = true;
        std::cout << "Watching directory: " << path_to_watch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            auto previous_files = current_files;
            populate_file_set();

            // Detect new files
            for (const auto& filename : current_files) {
                if (previous_files.find(filename) == previous_files.end()) {
                    std::cout << "[+] New file detected: " << filename << std::endl;
                }
            }

            // Detect deleted files
            for (const auto& filename : previous_files) {
                if (current_files.find(filename) == current_files.end()) {
                    std::cout << "[-] File deleted: " << filename << std::endl;
                }
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
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <mutex>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using FileChangeCallback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher() : running_(false) {}

    void addWatchPath(const fs::path& path, bool recursive = true) {
        std::lock_guard<std::mutex> lock(mutex_);
        watch_paths_.push_back({path, recursive});
        scanExistingFiles(path, recursive);
    }

    void setChangeCallback(FileChangeCallback callback) {
        callback_ = std::move(callback);
    }

    void start() {
        running_ = true;
        monitor_thread_ = std::thread(&FileSystemWatcher::monitorLoop, this);
    }

    void stop() {
        running_ = false;
        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

private:
    struct WatchInfo {
        fs::path path;
        bool recursive;
    };

    void scanExistingFiles(const fs::path& path, bool recursive) {
        try {
            if (recursive) {
                for (const auto& entry : fs::recursive_directory_iterator(path)) {
                    if (entry.is_regular_file()) {
                        file_timestamps_[entry.path()] = entry.last_write_time();
                    }
                }
            } else {
                for (const auto& entry : fs::directory_iterator(path)) {
                    if (entry.is_regular_file()) {
                        file_timestamps_[entry.path()] = entry.last_write_time();
                    }
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error during scan: " << e.what() << std::endl;
        }
    }

    void monitorLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            checkForChanges();
        }
    }

    void checkForChanges() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (const auto& watch_info : watch_paths_) {
            try {
                if (watch_info.recursive) {
                    checkDirectoryRecursive(watch_info.path);
                } else {
                    checkDirectory(watch_info.path);
                }
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Filesystem error: " << e.what() << std::endl;
            }
        }
    }

    void checkDirectory(const fs::path& dir) {
        std::unordered_map<fs::path, fs::file_time_type> current_files;

        for (const auto& entry : fs::directory_iterator(dir)) {
            if (entry.is_regular_file()) {
                current_files[entry.path()] = entry.last_write_time();
            }
        }

        detectChanges(current_files);
    }

    void checkDirectoryRecursive(const fs::path& dir) {
        std::unordered_map<fs::path, fs::file_time_type> current_files;

        for (const auto& entry : fs::recursive_directory_iterator(dir)) {
            if (entry.is_regular_file()) {
                current_files[entry.path()] = entry.last_write_time();
            }
        }

        detectChanges(current_files);
    }

    void detectChanges(const std::unordered_map<fs::path, fs::file_time_type>& current_files) {
        for (const auto& [path, timestamp] : current_files) {
            auto it = file_timestamps_.find(path);
            if (it == file_timestamps_.end()) {
                file_timestamps_[path] = timestamp;
                if (callback_) {
                    callback_(path, "created");
                }
            } else if (it->second != timestamp) {
                it->second = timestamp;
                if (callback_) {
                    callback_(path, "modified");
                }
            }
        }

        auto it = file_timestamps_.begin();
        while (it != file_timestamps_.end()) {
            if (current_files.find(it->first) == current_files.end()) {
                if (callback_) {
                    callback_(it->first, "deleted");
                }
                it = file_timestamps_.erase(it);
            } else {
                ++it;
            }
        }
    }

    std::vector<WatchInfo> watch_paths_;
    std::unordered_map<fs::path, fs::file_time_type> file_timestamps_;
    FileChangeCallback callback_;
    std::atomic<bool> running_;
    std::thread monitor_thread_;
    std::mutex mutex_;
};