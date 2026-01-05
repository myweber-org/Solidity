
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
            snapshotDirectory();
        }
    }

    void startWatching(int interval_seconds = 2) {
        running = true;
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
    std::string watch_path;
    bool running;
    std::unordered_map<std::string, fs::file_time_type> file_snapshot;

    void snapshotDirectory() {
        file_snapshot.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_snapshot[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void checkForChanges() {
        std::unordered_map<std::string, fs::file_time_type> current_state;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                std::string file_path = entry.path().string();
                current_state[file_path] = fs::last_write_time(entry.path());
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
    SimpleFileWatcher watcher(".");
    std::thread watch_thread([&watcher]() {
        watcher.startWatching(1);
    });

    std::cout << "Press Enter to stop watching..." << std::endl;
    std::cin.get();
    watcher.stopWatching();
    watch_thread.join();

    return 0;
}