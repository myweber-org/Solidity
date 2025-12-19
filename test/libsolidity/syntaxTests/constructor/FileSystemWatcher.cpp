
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
            if (entry.is_regular_file()) {
                current_snapshot.insert(fs::canonical(entry.path()));
            }
        }
    }

    void watch_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            auto previous_snapshot = current_snapshot;
            update_snapshot();

            std::unordered_set<fs::path> added_files;
            std::unordered_set<fs::path> removed_files;

            for (const auto& file : current_snapshot) {
                if (previous_snapshot.find(file) == previous_snapshot.end()) {
                    added_files.insert(file);
                }
            }

            for (const auto& file : previous_snapshot) {
                if (current_snapshot.find(file) == current_snapshot.end()) {
                    removed_files.insert(file);
                }
            }

            if (!added_files.empty() || !removed_files.empty()) {
                std::lock_guard<std::mutex> lock(console_mutex);
                std::cout << "File system changes detected:" << std::endl;
                for (const auto& file : added_files) {
                    std::cout << "  [+] " << file.filename() << std::endl;
                }
                for (const auto& file : removed_files) {
                    std::cout << "  [-] " << file.filename() << std::endl;
                }
                std::cout << std::endl;
            }
        }
    }

    fs::path watch_path;
    std::unordered_set<fs::path> current_snapshot;
    std::atomic<bool> running;
    std::thread watcher_thread;
    std::mutex console_mutex;
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        std::cout << "Watching current directory for file changes..." << std::endl;
        std::cout << "Press Enter to stop watching." << std::endl;
        
        watcher.start();
        std::cin.get();
        watcher.stop();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}