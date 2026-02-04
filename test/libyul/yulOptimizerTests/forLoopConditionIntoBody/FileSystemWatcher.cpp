#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>
#include <atomic>

namespace fs = std::filesystem;

class FileWatcher {
public:
    FileWatcher(const fs::path& path) : watch_path(path), running(false) {
        if (!fs::exists(path) || !fs::is_directory(path)) {
            throw std::runtime_error("Path does not exist or is not a directory");
        }
        update_snapshot();
    }

    void start() {
        running = true;
        watcher_thread = std::thread(&FileWatcher::watch_loop, this);
    }

    void stop() {
        running = false;
        if (watcher_thread.joinable()) {
            watcher_thread.join();
        }
    }

    ~FileWatcher() {
        stop();
    }

private:
    fs::path watch_path;
    std::atomic<bool> running;
    std::thread watcher_thread;
    std::unordered_set<std::string> file_snapshot;

    void update_snapshot() {
        file_snapshot.clear();
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                file_snapshot.insert(entry.path().filename().string());
            }
        }
    }

    void watch_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            std::unordered_set<std::string> current_files;
            for (const auto& entry : fs::directory_iterator(watch_path)) {
                if (entry.is_regular_file()) {
                    current_files.insert(entry.path().filename().string());
                }
            }

            for (const auto& file : current_files) {
                if (file_snapshot.find(file) == file_snapshot.end()) {
                    std::cout << "New file detected: " << file << std::endl;
                }
            }

            for (const auto& file : file_snapshot) {
                if (current_files.find(file) == current_files.end()) {
                    std::cout << "File removed: " << file << std::endl;
                }
            }

            file_snapshot = std::move(current_files);
        }
    }
};

int main() {
    try {
        FileWatcher watcher(".");
        watcher.start();

        std::cout << "Watching current directory. Press Enter to stop..." << std::endl;
        std::cin.get();

        watcher.stop();
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
#include <string>
#include <functional>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    using FileChangeCallback = std::function<void(const fs::path&, const std::string&)>;

    SimpleFileWatcher(const fs::path& watch_path, FileChangeCallback callback)
        : watch_path_(watch_path), callback_(callback), running_(false) {
        if (!fs::exists(watch_path)) {
            throw std::runtime_error("Watch path does not exist");
        }
        scanCurrentState();
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
    void scanCurrentState() {
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
                        callback_(path, "CREATED");
                    }
                } else if (it->second != last_write) {
                    if (callback_) {
                        callback_(path, "MODIFIED");
                    }
                }
            }
        }

        for (const auto& old_pair : file_states_) {
            if (current_states.find(old_pair.first) == current_states.end()) {
                if (callback_) {
                    callback_(old_pair.first, "DELETED");
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
    std::cout << "File: " << path.filename() << " - Action: " << change_type << std::endl;
}

int main() {
    try {
        fs::path watch_dir = ".";
        SimpleFileWatcher watcher(watch_dir, exampleCallback);
        
        std::cout << "Watching directory: " << fs::absolute(watch_dir) << std::endl;
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