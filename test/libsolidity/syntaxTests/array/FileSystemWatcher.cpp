
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

    SimpleFileWatcher(const std::string& path) : watch_path(path), running(false) {
        if (!fs::exists(path)) {
            throw std::runtime_error("Path does not exist: " + path);
        }
    }

    void start() {
        running = true;
        snapshot = take_snapshot();
        std::cout << "Watching directory: " << watch_path << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            check_for_changes();
        }
    }

    void stop() {
        running = false;
    }

private:
    FileTimeMap take_snapshot() {
        FileTimeMap current_state;
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                current_state[entry.path().string()] = fs::last_write_time(entry);
            }
        }
        return current_state;
    }

    void check_for_changes() {
        auto current_state = take_snapshot();

        for (const auto& [path, time] : current_state) {
            auto it = snapshot.find(path);
            if (it == snapshot.end()) {
                std::cout << "[ADDED] " << path << std::endl;
            } else if (it->second != time) {
                std::cout << "[MODIFIED] " << path << std::endl;
            }
        }

        for (const auto& [path, time] : snapshot) {
            if (current_state.find(path) == current_state.end()) {
                std::cout << "[DELETED] " << path << std::endl;
            }
        }

        snapshot = std::move(current_state);
    }

    std::string watch_path;
    FileTimeMap snapshot;
    bool running;
};

int main() {
    try {
        SimpleFileWatcher watcher(".");
        watcher.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}