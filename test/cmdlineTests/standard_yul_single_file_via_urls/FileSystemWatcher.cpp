
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
        if (running) return;
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
        std::unordered_set<std::string> files;
        if (!fs::exists(watch_path)) {
            return files;
        }
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                files.insert(fs::relative(entry.path(), watch_path).string());
            }
        }
        return files;
    }

    void watch_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            auto current_snapshot = take_snapshot();

            std::unordered_set<std::string> added_files;
            std::unordered_set<std::string> removed_files;

            for (const auto& file : current_snapshot) {
                if (snapshot.find(file) == snapshot.end()) {
                    added_files.insert(file);
                }
            }

            for (const auto& file : snapshot) {
                if (current_snapshot.find(file) == current_snapshot.end()) {
                    removed_files.insert(file);
                }
            }

            if (!added_files.empty() || !removed_files.empty()) {
                std::cout << "File system changes detected in: " << watch_path << std::endl;
                for (const auto& file : added_files) {
                    std::cout << "  [+] " << file << std::endl;
                }
                for (const auto& file : removed_files) {
                    std::cout << "  [-] " << file << std::endl;
                }
                std::cout << std::endl;
                snapshot = std::move(current_snapshot);
            }
        }
    }
};

int main() {
    fs::path path_to_watch = "./test_watch";
    fs::create_directories(path_to_watch);

    FileSystemWatcher watcher(path_to_watch);
    watcher.start();

    std::cout << "Watching directory: " << fs::absolute(path_to_watch) << std::endl;
    std::cout << "Press Enter to stop watching..." << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(10));

    fs::path new_file = path_to_watch / "example.txt";
    std::ofstream outfile(new_file);
    outfile << "Test content" << std::endl;
    outfile.close();

    std::this_thread::sleep_for(std::chrono::seconds(3));

    fs::remove(new_file);

    std::this_thread::sleep_for(std::chrono::seconds(3));

    watcher.stop();
    fs::remove_all(path_to_watch);

    return 0;
}