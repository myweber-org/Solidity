
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
}