
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <vector>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher() : running_(false) {}

    void addWatchPath(const fs::path& path) {
        if (fs::exists(path) && fs::is_directory(path)) {
            watch_paths_.push_back(fs::canonical(path));
        }
    }

    void setEventCallback(Callback callback) {
        callback_ = std::move(callback);
    }

    void start() {
        if (running_) return;

        running_ = true;
        snapshotCurrentState();
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
    void snapshotCurrentState() {
        file_states_.clear();
        for (const auto& watch_path : watch_paths_) {
            for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
                if (entry.is_regular_file()) {
                    auto last_write = fs::last_write_time(entry);
                    file_states_[entry.path()] = last_write;
                }
            }
        }
    }

    void monitorLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            std::unordered_map<fs::path, fs::file_time_type> current_states;

            for (const auto& watch_path : watch_paths_) {
                try {
                    for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
                        if (entry.is_regular_file()) {
                            auto path = entry.path();
                            auto last_write = fs::last_write_time(entry);
                            current_states[path] = last_write;

                            auto it = file_states_.find(path);
                            if (it == file_states_.end()) {
                                if (callback_) callback_(path, "created");
                            } else if (it->second != last_write) {
                                if (callback_) callback_(path, "modified");
                            }
                        }
                    }
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Filesystem error: " << e.what() << std::endl;
                }
            }

            for (const auto& [path, _] : file_states_) {
                if (current_states.find(path) == current_states.end()) {
                    if (callback_) callback_(path, "deleted");
                }
            }

            file_states_.swap(current_states);
        }
    }

    std::vector<fs::path> watch_paths_;
    std::unordered_map<fs::path, fs::file_time_type> file_states_;
    Callback callback_;
    std::thread monitor_thread_;
    std::atomic<bool> running_;
};

void exampleUsage() {
    FileSystemWatcher watcher;
    
    watcher.addWatchPath(".");
    watcher.addWatchPath("../parent_dir");

    watcher.setEventCallback([](const fs::path& path, const std::string& action) {
        std::cout << "File: " << path << " Action: " << action << std::endl;
    });

    watcher.start();

    std::this_thread::sleep_for(std::chrono::seconds(30));
    watcher.stop();
}

int main() {
    exampleUsage();
    return 0;
}
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const fs::path& path_to_watch) : path_to_watch_(path_to_watch) {
        for (const auto& entry : fs::recursive_directory_iterator(path_to_watch_)) {
            if (fs::is_regular_file(entry.path())) {
                known_files_.insert(fs::canonical(entry.path()));
            }
        }
    }

    void startMonitoring(int interval_seconds = 1) {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            checkForChanges();
        }
    }

private:
    void checkForChanges() {
        std::unordered_set<std::string> current_files;

        for (const auto& entry : fs::recursive_directory_iterator(path_to_watch_)) {
            if (fs::is_regular_file(entry.path())) {
                current_files.insert(fs::canonical(entry.path()));
            }
        }

        for (const auto& file : current_files) {
            if (known_files_.find(file) == known_files_.end()) {
                std::cout << "File added: " << file << std::endl;
            }
        }

        for (const auto& file : known_files_) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "File removed: " << file << std::endl;
            }
        }

        known_files_ = std::move(current_files);
    }

    fs::path path_to_watch_;
    std::unordered_set<std::string> known_files_;
};

int main() {
    std::string path;
    std::cout << "Enter directory path to monitor: ";
    std::getline(std::cin, path);

    if (!fs::exists(path) || !fs::is_directory(path)) {
        std::cerr << "Invalid directory path." << std::endl;
        return 1;
    }

    FileSystemWatcher watcher(path);
    std::cout << "Monitoring directory: " << fs::canonical(path) << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;

    watcher.startMonitoring();

    return 0;
}