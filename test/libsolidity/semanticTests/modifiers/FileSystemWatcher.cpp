
#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <unordered_set>
#include <atomic>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const fs::path& path) : watch_path(path), running(false) {}

    void start() {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            std::cerr << "Path does not exist or is not a directory." << std::endl;
            return;
        }

        running = true;
        snapshot = take_snapshot();
        watcher_thread = std::thread(&FileSystemWatcher::watch_loop, this);
    }

    void stop() {
        running = false;
        if (watcher_thread.joinable()) {
            watcher_thread.join();
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

private:
    fs::path watch_path;
    std::atomic<bool> running;
    std::thread watcher_thread;
    std::unordered_set<std::string> snapshot;

    std::unordered_set<std::string> take_snapshot() {
        std::unordered_set<std::string> current_files;
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            current_files.insert(entry.path().string());
        }
        return current_files;
    }

    void watch_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            auto current_snapshot = take_snapshot();

            // Detect new files
            for (const auto& file : current_snapshot) {
                if (snapshot.find(file) == snapshot.end()) {
                    std::cout << "New file detected: " << file << std::endl;
                }
            }

            // Detect deleted files
            for (const auto& file : snapshot) {
                if (current_snapshot.find(file) == current_snapshot.end()) {
                    std::cout << "File deleted: " << file << std::endl;
                }
            }

            snapshot = std::move(current_snapshot);
        }
    }
};

int main() {
    FileSystemWatcher watcher(".");
    watcher.start();

    // Keep the watcher running for 30 seconds for demonstration
    std::this_thread::sleep_for(std::chrono::seconds(30));

    watcher.stop();
    return 0;
}