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
            throw std::runtime_error("Provided path is not a valid directory.");
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
    fs::path watch_path;
    std::unordered_set<std::string> file_snapshot;
    std::atomic<bool> running;
    std::thread watcher_thread;

    void update_snapshot() {
        file_snapshot.clear();
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                file_snapshot.insert(entry.path().filename().string());
            }
        }
    }

    void watch_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            std::unordered_set<std::string> current_files;
            for (const auto& entry : fs::directory_iterator(watch_path)) {
                if (entry.is_regular_file()) {
                    current_files.insert(entry.path().filename().string());
                }
            }

            for (const auto& file : current_files) {
                if (file_snapshot.find(file) == file_snapshot.end()) {
                    std::cout << "New file detected: " << file << std::endl;
                }
            }

            for (const auto& file : file_snapshot) {
                if (current_files.find(file) == current_files.end()) {
                    std::cout << "File removed: " << file << std::endl;
                }
            }

            file_snapshot = std::move(current_files);
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start();

        std::cout << "Watching current directory. Press Enter to stop..." << std::endl;
        std::cin.get();

        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}