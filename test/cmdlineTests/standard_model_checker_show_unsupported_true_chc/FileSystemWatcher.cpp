#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    SimpleFileWatcher(const std::string& path) : watch_path(path), running(false) {
        if (fs::exists(path) && fs::is_directory(path)) {
            refresh_file_map();
        }
    }

    void start(int interval_seconds = 2) {
        running = true;
        std::cout << "Watching directory: " << watch_path << std::endl;
        
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

    void stop() {
        running = false;
    }

private:
    std::string watch_path;
    bool running;
    std::unordered_map<std::string, fs::file_time_type> file_map;

    void refresh_file_map() {
        file_map.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_map[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void check_for_changes() {
        auto current_files = file_map;
        refresh_file_map();

        for (const auto& [path, mtime] : file_map) {
            auto old_it = current_files.find(path);
            if (old_it == current_files.end()) {
                std::cout << "[NEW] " << path << std::endl;
            } else if (old_it->second != mtime) {
                std::cout << "[MODIFIED] " << path << std::endl;
            }
        }

        for (const auto& [path, mtime] : current_files) {
            if (file_map.find(path) == file_map.end()) {
                std::cout << "[DELETED] " << path << std::endl;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    SimpleFileWatcher watcher(argv[1]);
    
    std::thread watch_thread([&watcher]() {
        watcher.start();
    });

    std::cout << "Press Enter to stop watching..." << std::endl;
    std::cin.get();
    
    watcher.stop();
    watch_thread.join();
    
    return 0;
}
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    using FileTimeMap = std::unordered_map<std::string, fs::file_time_type>;

    SimpleFileWatcher(const std::string& path, int interval_ms = 1000)
        : watch_path_(path), interval_ms_(interval_ms), running_(false) {}

    void start() {
        running_ = true;
        snapshot_ = take_snapshot();
        std::cout << "Watching directory: " << watch_path_ << std::endl;

        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms_));
            check_for_changes();
        }
    }

    void stop() {
        running_ = false;
    }

private:
    FileTimeMap take_snapshot() {
        FileTimeMap snapshot;
        try {
            for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
                if (entry.is_regular_file()) {
                    snapshot[entry.path().string()] = fs::last_write_time(entry);
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
        return snapshot;
    }

    void check_for_changes() {
        auto current_snapshot = take_snapshot();

        for (const auto& [path, time] : current_snapshot) {
            auto old_it = snapshot_.find(path);
            if (old_it == snapshot_.end()) {
                std::cout << "[NEW] " << path << std::endl;
            } else if (old_it->second != time) {
                std::cout << "[MODIFIED] " << path << std::endl;
            }
        }

        for (const auto& [path, time] : snapshot_) {
            if (current_snapshot.find(path) == current_snapshot.end()) {
                std::cout << "[DELETED] " << path << std::endl;
            }
        }

        snapshot_ = std::move(current_snapshot);
    }

    std::string watch_path_;
    int interval_ms_;
    bool running_;
    FileTimeMap snapshot_;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    std::string watch_dir = argv[1];
    if (!fs::exists(watch_dir) || !fs::is_directory(watch_dir)) {
        std::cerr << "Invalid directory path: " << watch_dir << std::endl;
        return 1;
    }

    SimpleFileWatcher watcher(watch_dir, 2000);

    std::thread watch_thread([&watcher]() {
        watcher.start();
    });

    std::cout << "File watcher started. Press Enter to stop..." << std::endl;
    std::cin.get();

    watcher.stop();
    watch_thread.join();

    std::cout << "File watcher stopped." << std::endl;
    return 0;
}