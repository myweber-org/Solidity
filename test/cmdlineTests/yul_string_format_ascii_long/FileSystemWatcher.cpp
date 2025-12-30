
#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <unordered_set>
#include <atomic>

namespace fs = std::filesystem;

class DirectoryWatcher {
public:
    DirectoryWatcher(const fs::path& dir_path) : watch_path(dir_path), running(false) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Provided path is not a valid directory.");
        }
        update_snapshot();
    }

    void start() {
        running = true;
        watcher_thread = std::thread(&DirectoryWatcher::watch_loop, this);
    }

    void stop() {
        running = false;
        if (watcher_thread.joinable()) {
            watcher_thread.join();
        }
    }

    ~DirectoryWatcher() {
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
            if (fs::is_regular_file(entry.path())) {
                file_snapshot.insert(entry.path().filename().string());
            }
        }
    }

    void watch_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            std::unordered_set<std::string> current_files;
            for (const auto& entry : fs::directory_iterator(watch_path)) {
                if (fs::is_regular_file(entry.path())) {
                    current_files.insert(entry.path().filename().string());
                }
            }

            for (const auto& file : current_files) {
                if (file_snapshot.find(file) == file_snapshot.end()) {
                    std::cout << "File added: " << file << std::endl;
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
        DirectoryWatcher watcher("./test_directory");
        std::cout << "Watching directory: ./test_directory" << std::endl;
        std::cout << "Press Enter to stop watching..." << std::endl;

        watcher.start();
        std::cin.get();
        watcher.stop();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}