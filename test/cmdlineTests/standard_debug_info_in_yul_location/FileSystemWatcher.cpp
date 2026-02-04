#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>
#include <mutex>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& path) : watch_path(path), running(false) {
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
    std::mutex snapshot_mutex;
    std::thread watcher_thread;
    bool running;

    void update_snapshot() {
        std::lock_guard<std::mutex> lock(snapshot_mutex);
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

            std::lock_guard<std::mutex> lock(snapshot_mutex);
            std::unordered_set<std::string> added_files, removed_files;

            for (const auto& file : current_files) {
                if (file_snapshot.find(file) == file_snapshot.end()) {
                    added_files.insert(file);
                }
            }

            for (const auto& file : file_snapshot) {
                if (current_files.find(file) == current_files.end()) {
                    removed_files.insert(file);
                }
            }

            if (!added_files.empty() || !removed_files.empty()) {
                std::cout << "File system change detected in: " << watch_path << std::endl;
                for (const auto& file : added_files) {
                    std::cout << "  [+] " << file << std::endl;
                }
                for (const auto& file : removed_files) {
                    std::cout << "  [-] " << file << std::endl;
                }
                file_snapshot = std::move(current_files);
            }
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