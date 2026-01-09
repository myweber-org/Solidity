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

    ~FileSystemWatcher() {
        stop();
    }

    void addWatchPath(const fs::path& path, Callback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (fs::exists(path) && fs::is_directory(path)) {
            watch_paths_[path] = {callback, getCurrentFileState(path)};
        }
    }

    void start() {
        if (running_) return;
        running_ = true;
        watcher_thread_ = std::thread(&FileSystemWatcher::watchLoop, this);
    }

    void stop() {
        running_ = false;
        if (watcher_thread_.joinable()) {
            watcher_thread_.join();
        }
    }

private:
    struct WatchInfo {
        Callback callback;
        std::unordered_map<std::string, fs::file_time_type> file_states;
    };

    std::unordered_map<fs::path, WatchInfo> watch_paths_;
    std::atomic<bool> running_;
    std::thread watcher_thread_;
    std::mutex mutex_;

    std::unordered_map<std::string, fs::file_time_type> getCurrentFileState(const fs::path& directory) {
        std::unordered_map<std::string, fs::file_time_type> states;
        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (fs::is_regular_file(entry.path())) {
                states[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
        return states;
    }

    void checkForChanges(const fs::path& path, WatchInfo& info) {
        auto current_states = getCurrentFileState(path);
        auto& previous_states = info.file_states;

        for (const auto& [file_path, current_time] : current_states) {
            auto it = previous_states.find(file_path);
            if (it == previous_states.end()) {
                info.callback(file_path, "CREATED");
            } else if (it->second != current_time) {
                info.callback(file_path, "MODIFIED");
            }
        }

        for (const auto& [file_path, _] : previous_states) {
            if (current_states.find(file_path) == current_states.end()) {
                info.callback(file_path, "DELETED");
            }
        }

        info.file_states = std::move(current_states);
    }

    void watchLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto& [path, info] : watch_paths_) {
                checkForChanges(path, info);
            }
        }
    }
};

int main() {
    FileSystemWatcher watcher;

    watcher.addWatchPath("./test_directory", [](const fs::path& path, const std::string& action) {
        std::cout << "File: " << path << " Action: " << action << std::endl;
    });

    std::cout << "Watching for file changes in ./test_directory" << std::endl;
    std::cout << "Press Enter to stop watching..." << std::endl;

    watcher.start();
    std::cin.get();
    watcher.stop();

    return 0;
}#include <iostream>
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
        if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                current_files.insert(entry.path().filename().string());
            }
        }
    }

public:
    explicit FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        populate_file_set();
    }

    void start_watching(int interval_seconds = 2) {
        running = true;
        std::cout << "Watching directory: " << path_to_watch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
                std::cerr << "Directory does not exist or is not accessible." << std::endl;
                continue;
            }

            std::unordered_set<std::string> new_files;
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                new_files.insert(entry.path().filename().string());
            }

            for (const auto& file : new_files) {
                if (current_files.find(file) == current_files.end()) {
                    std::cout << "File added: " << file << std::endl;
                }
            }

            for (const auto& file : current_files) {
                if (new_files.find(file) == new_files.end()) {
                    std::cout << "File removed: " << file << std::endl;
                }
            }

            current_files = std::move(new_files);
        }
    }

    void stop_watching() {
        running = false;
    }
};

int main() {
    FileSystemWatcher watcher(".");
    watcher.start_watching();
    return 0;
}