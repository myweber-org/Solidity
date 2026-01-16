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
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher() : running_(false) {}

    void addWatchPath(const fs::path& path, Callback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (fs::exists(path) && fs::is_directory(path)) {
            watch_paths_[path] = {callback, getCurrentSnapshot(path)};
        }
    }

    void start() {
        running_ = true;
        monitor_thread_ = std::thread(&FileSystemWatcher::monitor, this);
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
        Callback callback;
        std::unordered_map<std::string, fs::file_time_type> snapshot;
    };

    std::atomic<bool> running_;
    std::thread monitor_thread_;
    std::mutex mutex_;
    std::unordered_map<fs::path, WatchInfo> watch_paths_;

    std::unordered_map<std::string, fs::file_time_type> getCurrentSnapshot(const fs::path& path) {
        std::unordered_map<std::string, fs::file_time_type> snapshot;
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (fs::is_regular_file(entry.status())) {
                snapshot[entry.path().string()] = fs::last_write_time(entry);
            }
        }
        return snapshot;
    }

    void monitor() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto& [path, info] : watch_paths_) {
                auto current_snapshot = getCurrentSnapshot(path);
                
                for (const auto& [file_path, current_time] : current_snapshot) {
                    auto it = info.snapshot.find(file_path);
                    if (it == info.snapshot.end()) {
                        info.callback(file_path, "CREATED");
                    } else if (it->second != current_time) {
                        info.callback(file_path, "MODIFIED");
                    }
                }

                for (const auto& [file_path, old_time] : info.snapshot) {
                    if (current_snapshot.find(file_path) == current_snapshot.end()) {
                        info.callback(file_path, "DELETED");
                    }
                }

                info.snapshot = std::move(current_snapshot);
            }
        }
    }
};

void exampleCallback(const fs::path& path, const std::string& action) {
    std::cout << "File: " << path << " Action: " << action << std::endl;
}

int main() {
    FileSystemWatcher watcher;
    
    watcher.addWatchPath("./watch_directory", exampleCallback);
    
    std::cout << "Starting file system watcher. Monitoring ./watch_directory" << std::endl;
    std::cout << "Press Enter to stop..." << std::endl;
    
    watcher.start();
    
    std::cin.get();
    
    watcher.stop();
    
    return 0;
}
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

    std::unordered_set<std::string> get_files_in_directory() {
        std::unordered_set<std::string> files;
        if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                if (fs::is_regular_file(entry.status())) {
                    files.insert(entry.path().filename().string());
                }
            }
        }
        return files;
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            current_files = get_files_in_directory();
            std::cout << "Watching directory: " << fs::absolute(path_to_watch) << std::endl;
        } else {
            std::cerr << "Path does not exist or is not a directory." << std::endl;
        }
    }

    void start_monitoring(int interval_seconds = 2) {
        std::cout << "Starting monitoring. Press Ctrl+C to stop." << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            auto new_files = get_files_in_directory();

            // Check for added files
            for (const auto& file : new_files) {
                if (current_files.find(file) == current_files.end()) {
                    std::cout << "File added: " << file << std::endl;
                }
            }

            // Check for removed files
            for (const auto& file : current_files) {
                if (new_files.find(file) == new_files.end()) {
                    std::cout << "File removed: " << file << std::endl;
                }
            }

            current_files = new_files;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    std::string directory_path = argv[1];
    FileSystemWatcher watcher(directory_path);
    watcher.start_monitoring();

    return 0;
}