
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

    SimpleFileWatcher(const std::string& path) : watch_path(path), running(false) {}

    void start() {
        if (!fs::exists(watch_path)) {
            std::cerr << "Path does not exist: " << watch_path << std::endl;
            return;
        }

        running = true;
        snapshot_files();
        std::cout << "Watching directory: " << watch_path << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            check_for_changes();
        }
    }

    void stop() {
        running = false;
    }

private:
    std::string watch_path;
    bool running;
    FileTimeMap file_timestamps;

    void snapshot_files() {
        file_timestamps.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                std::string file_path = entry.path().string();
                file_timestamps[file_path] = fs::last_write_time(entry);
            }
        }
    }

    void check_for_changes() {
        FileTimeMap current_state;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                std::string file_path = entry.path().string();
                current_state[file_path] = fs::last_write_time(entry);

                auto old_it = file_timestamps.find(file_path);
                if (old_it == file_timestamps.end()) {
                    std::cout << "[NEW] " << file_path << std::endl;
                } else if (old_it->second != current_state[file_path]) {
                    std::cout << "[MODIFIED] " << file_path << std::endl;
                }
            }
        }

        for (const auto& old_file : file_timestamps) {
            if (current_state.find(old_file.first) == current_state.end()) {
                std::cout << "[DELETED] " << old_file.first << std::endl;
            }
        }

        file_timestamps = std::move(current_state);
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    SimpleFileWatcher watcher(argv[1]);
    watcher.start();

    return 0;
}#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& path) : watch_path(path) {
        if (fs::exists(watch_path) && fs::is_directory(watch_path)) {
            populateSnapshot();
        } else {
            std::cerr << "Path does not exist or is not a directory: " << watch_path << std::endl;
        }
    }

    void startWatching(int interval_seconds = 1) {
        std::cout << "Watching directory: " << watch_path << std::endl;
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            checkForChanges();
        }
    }

    void stopWatching() {
        running = false;
    }

private:
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_snapshot;
    bool running = true;

    void populateSnapshot() {
        file_snapshot.clear();
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.status())) {
                file_snapshot[entry.path().filename().string()] = fs::last_write_time(entry);
            }
        }
    }

    void checkForChanges() {
        std::unordered_map<std::string, fs::file_time_type> current_state;

        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.status())) {
                std::string filename = entry.path().filename().string();
                current_state[filename] = fs::last_write_time(entry);
            }
        }

        for (const auto& [file, mtime] : current_state) {
            if (file_snapshot.find(file) == file_snapshot.end()) {
                std::cout << "File added: " << file << std::endl;
            } else if (file_snapshot[file] != mtime) {
                std::cout << "File modified: " << file << std::endl;
            }
        }

        for (const auto& [file, mtime] : file_snapshot) {
            if (current_state.find(file) == current_state.end()) {
                std::cout << "File deleted: " << file << std::endl;
            }
        }

        file_snapshot = std::move(current_state);
    }
};

int main() {
    fs::path path_to_watch = "./test_watch";
    FileSystemWatcher watcher(path_to_watch);
    watcher.startWatching(2);
    return 0;
}