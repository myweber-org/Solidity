
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
    using FilePath = fs::path;
    using FileTime = fs::file_time_type;
    using Callback = std::function<void(const FilePath&, const std::string&)>;

    FileSystemWatcher() = default;

    void addWatchPath(const FilePath& path) {
        if (fs::exists(path)) {
            updateFileState(path);
            watchPaths.insert(path);
        }
    }

    void setCallback(Callback cb) {
        callback = std::move(cb);
    }

    void startMonitoring(int intervalMs = 1000) {
        monitoring = true;
        while (monitoring) {
            checkForChanges();
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
        }
    }

    void stopMonitoring() {
        monitoring = false;
    }

private:
    std::unordered_map<FilePath, FileTime> fileStates;
    std::unordered_set<FilePath> watchPaths;
    Callback callback;
    bool monitoring = false;

    void updateFileState(const FilePath& path) {
        if (fs::exists(path)) {
            fileStates[path] = fs::last_write_time(path);
        }
    }

    void checkForChanges() {
        for (const auto& path : watchPaths) {
            if (!fs::exists(path)) {
                if (fileStates.find(path) != fileStates.end()) {
                    fileStates.erase(path);
                    if (callback) {
                        callback(path, "deleted");
                    }
                }
                continue;
            }

            auto currentTime = fs::last_write_time(path);
            auto it = fileStates.find(path);

            if (it == fileStates.end()) {
                fileStates[path] = currentTime;
                if (callback) {
                    callback(path, "created");
                }
            } else if (it->second != currentTime) {
                it->second = currentTime;
                if (callback) {
                    callback(path, "modified");
                }
            }
        }
    }
};

int main() {
    FileSystemWatcher watcher;

    watcher.setCallback([](const FileSystemWatcher::FilePath& path, const std::string& action) {
        std::cout << "File: " << path.string() << " - Action: " << action << std::endl;
    });

    watcher.addWatchPath("test_directory");
    watcher.addWatchPath("example.txt");

    std::cout << "Starting file system watcher. Monitoring for changes..." << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;

    std::thread monitorThread([&watcher]() {
        watcher.startMonitoring(500);
    });

    monitorThread.join();

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
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher() : running_(false) {}

    void addWatchPath(const fs::path& path, Callback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (fs::exists(path) && fs::is_directory(path)) {
            watch_paths_[path] = callback;
            updateSnapshot(path);
        } else {
            std::cerr << "Path does not exist or is not a directory: " << path << std::endl;
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
    void updateSnapshot(const fs::path& path) {
        auto& snapshot = file_snapshots_[path];
        snapshot.clear();
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (fs::is_regular_file(entry.status())) {
                snapshot[entry.path()] = fs::last_write_time(entry);
            }
        }
    }

    void monitor() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto& [path, callback] : watch_paths_) {
                checkForChanges(path, callback);
            }
        }
    }

    void checkForChanges(const fs::path& path, Callback callback) {
        auto& snapshot = file_snapshots_[path];
        std::unordered_map<fs::path, fs::file_time_type> current_state;

        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (fs::is_regular_file(entry.status())) {
                current_state[entry.path()] = fs::last_write_time(entry);
            }
        }

        for (const auto& [file_path, current_time] : current_state) {
            auto it = snapshot.find(file_path);
            if (it == snapshot.end()) {
                callback(file_path, "created");
                snapshot[file_path] = current_time;
            } else if (it->second != current_time) {
                callback(file_path, "modified");
                it->second = current_time;
            }
        }

        for (auto it = snapshot.begin(); it != snapshot.end();) {
            if (current_state.find(it->first) == current_state.end()) {
                callback(it->first, "deleted");
                it = snapshot.erase(it);
            } else {
                ++it;
            }
        }
    }

    std::atomic<bool> running_;
    std::thread monitor_thread_;
    std::mutex mutex_;
    std::unordered_map<fs::path, Callback> watch_paths_;
    std::unordered_map<fs::path, std::unordered_map<fs::path, fs::file_time_type>> file_snapshots_;
};

int main() {
    FileSystemWatcher watcher;

    watcher.addWatchPath(fs::current_path(), [](const fs::path& path, const std::string& action) {
        std::cout << "File " << path.filename() << " has been " << action << std::endl;
    });

    std::cout << "Watching directory: " << fs::current_path() << std::endl;
    std::cout << "Press Enter to stop watching..." << std::endl;

    watcher.start();
    std::cin.get();
    watcher.stop();

    return 0;
}