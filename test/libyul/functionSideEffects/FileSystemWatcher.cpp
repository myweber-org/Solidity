
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
        : watch_directory(watch_path), callback(callback), running(false) {
        if (!fs::exists(watch_directory) || !fs::is_directory(watch_directory)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        initializeFileStates();
    }

    ~SimpleFileWatcher() {
        stop();
    }

    void start(int interval_ms = 1000) {
        running = true;
        watcher_thread = std::thread(&SimpleFileWatcher::watchLoop, this, interval_ms);
    }

    void stop() {
        running = false;
        if (watcher_thread.joinable()) {
            watcher_thread.join();
        }
    }

private:
    fs::path watch_directory;
    FileChangeCallback callback;
    std::unordered_map<std::string, fs::file_time_type> file_states;
    std::atomic<bool> running;
    std::thread watcher_thread;

    void initializeFileStates() {
        file_states.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                file_states[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void watchLoop(int interval_ms) {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
            checkForChanges();
        }
    }

    void checkForChanges() {
        std::unordered_map<std::string, fs::file_time_type> current_states;

        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                std::string file_path = entry.path().string();
                auto current_time = fs::last_write_time(entry);
                current_states[file_path] = current_time;

                auto it = file_states.find(file_path);
                if (it == file_states.end()) {
                    callback(entry.path(), "created");
                } else if (it->second != current_time) {
                    callback(entry.path(), "modified");
                }
            }
        }

        for (const auto& [old_path, old_time] : file_states) {
            if (current_states.find(old_path) == current_states.end()) {
                callback(fs::path(old_path), "deleted");
            }
        }

        file_states.swap(current_states);
    }
};

void exampleCallback(const fs::path& file_path, const std::string& change_type) {
    std::cout << "File: " << file_path << " - Action: " << change_type << std::endl;
}

int main() {
    try {
        fs::path current_path = fs::current_path();
        SimpleFileWatcher watcher(current_path, exampleCallback);

        std::cout << "Watching directory: " << current_path << std::endl;
        std::cout << "Press Enter to stop watching..." << std::endl;

        watcher.start(2000);

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
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path path_to_watch;
    std::unordered_set<std::string> current_files;
    bool running = false;

    void scan_directory() {
        std::unordered_set<std::string> new_files;
        for (const auto& entry : fs::directory_iterator(path_to_watch)) {
            if (entry.is_regular_file()) {
                new_files.insert(entry.path().filename().string());
            }
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

public:
    explicit FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
            throw std::runtime_error("Invalid directory path");
        }
        scan_directory();
    }

    void start_watching(int interval_seconds = 2) {
        running = true;
        std::cout << "Watching directory: " << path_to_watch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            try {
                scan_directory();
            } catch (const std::exception& e) {
                std::cerr << "Error scanning directory: " << e.what() << std::endl;
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
        watcher.start_watching();
    } catch (const std::exception& e) {
        std::cerr << "Failed to start watcher: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}