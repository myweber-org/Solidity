
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    SimpleFileWatcher(const fs::path& path) : watch_path(path), running(false) {
        if (!fs::exists(path) || !fs::is_directory(path)) {
            throw std::runtime_error("Path does not exist or is not a directory");
        }
        updateSnapshot();
    }

    void start(int interval_seconds = 1) {
        running = true;
        watcher_thread = std::thread(&SimpleFileWatcher::watchLoop, this, interval_seconds);
    }

    void stop() {
        running = false;
        if (watcher_thread.joinable()) {
            watcher_thread.join();
        }
    }

    ~SimpleFileWatcher() {
        stop();
    }

private:
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_snapshot;
    std::thread watcher_thread;
    bool running;

    void updateSnapshot() {
        file_snapshot.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_snapshot[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void watchLoop(int interval_seconds) {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            checkForChanges();
        }
    }

    void checkForChanges() {
        std::unordered_map<std::string, fs::file_time_type> current_state;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                std::string path_str = entry.path().string();
                current_state[path_str] = fs::last_write_time(entry.path());
            }
        }

        for (const auto& [path, old_time] : file_snapshot) {
            if (current_state.find(path) == current_state.end()) {
                std::cout << "File deleted: " << path << std::endl;
            } else if (current_state[path] != old_time) {
                std::cout << "File modified: " << path << std::endl;
            }
        }

        for (const auto& [path, new_time] : current_state) {
            if (file_snapshot.find(path) == file_snapshot.end()) {
                std::cout << "File created: " << path << std::endl;
            }
        }

        file_snapshot = std::move(current_state);
    }
};

int main() {
    try {
        SimpleFileWatcher watcher(".");
        watcher.start(2);

        std::cout << "Watching current directory. Press Enter to stop..." << std::endl;
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
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path directory_to_watch;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps;
    bool running = false;

    void populate_file_map() {
        file_timestamps.clear();
        for (const auto& entry : fs::directory_iterator(directory_to_watch)) {
            if (fs::is_regular_file(entry.status())) {
                file_timestamps[entry.path().filename().string()] = fs::last_write_time(entry);
            }
        }
    }

public:
    explicit FileSystemWatcher(const std::string& dir_path) : directory_to_watch(dir_path) {
        if (!fs::exists(directory_to_watch) || !fs::is_directory(directory_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        populate_file_map();
        std::cout << "Watching directory: " << directory_to_watch << std::endl;
    }

    void start(int interval_seconds = 1) {
        running = true;
        std::cout << "Starting watch loop (interval: " << interval_seconds << "s). Press Ctrl+C to stop." << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            for (const auto& entry : fs::directory_iterator(directory_to_watch)) {
                if (!fs::is_regular_file(entry.status())) {
                    continue;
                }

                std::string filename = entry.path().filename().string();
                auto current_write_time = fs::last_write_time(entry);

                if (file_timestamps.find(filename) == file_timestamps.end()) {
                    std::cout << "[NEW] File added: " << filename << std::endl;
                    file_timestamps[filename] = current_write_time;
                } else if (file_timestamps[filename] != current_write_time) {
                    std::cout << "[MODIFIED] File changed: " << filename << std::endl;
                    file_timestamps[filename] = current_write_time;
                }
            }

            for (auto it = file_timestamps.begin(); it != file_timestamps.end(); ) {
                fs::path file_path = directory_to_watch / it->first;
                if (!fs::exists(file_path)) {
                    std::cout << "[DELETED] File removed: " << it->first << std::endl;
                    it = file_timestamps.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    void stop() {
        running = false;
        std::cout << "Stopping file system watcher." << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start(2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}