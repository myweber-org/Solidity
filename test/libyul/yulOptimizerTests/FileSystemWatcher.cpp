#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <unordered_set>
#include <atomic>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const fs::path& path) : watch_path(path), running(false) {
        if (!fs::exists(path) || !fs::is_directory(path)) {
            throw std::runtime_error("Invalid directory path");
        }
        update_snapshot();
    }

    void start() {
        running = true;
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
    void update_snapshot() {
        current_snapshot.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                current_snapshot.insert(fs::canonical(entry.path()));
            }
        }
    }

    void watch_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            
            std::unordered_set<fs::path> new_snapshot;
            for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
                if (fs::is_regular_file(entry.path())) {
                    new_snapshot.insert(fs::canonical(entry.path()));
                }
            }

            for (const auto& file : new_snapshot) {
                if (current_snapshot.find(file) == current_snapshot.end()) {
                    std::cout << "[ADDED] " << file.filename() << std::endl;
                }
            }

            for (const auto& file : current_snapshot) {
                if (new_snapshot.find(file) == new_snapshot.end()) {
                    std::cout << "[REMOVED] " << file.filename() << std::endl;
                }
            }

            current_snapshot = std::move(new_snapshot);
        }
    }

    fs::path watch_path;
    std::unordered_set<fs::path> current_snapshot;
    std::thread watcher_thread;
    std::atomic<bool> running;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        std::cout << "Watching directory: " << fs::absolute(argv[1]) << std::endl;
        std::cout << "Press Enter to stop..." << std::endl;
        
        watcher.start();
        std::cin.get();
        watcher.stop();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}