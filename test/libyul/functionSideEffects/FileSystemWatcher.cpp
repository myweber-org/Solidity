
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
}