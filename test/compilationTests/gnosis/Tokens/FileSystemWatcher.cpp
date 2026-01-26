
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>
#include <condition_variable>
#include <mutex>

namespace fs = std::filesystem;

class FileWatcher {
public:
    FileWatcher(const fs::path& path_to_watch) : watch_path(path_to_watch) {
        if (fs::exists(watch_path) && fs::is_directory(watch_path)) {
            for (const auto& entry : fs::directory_iterator(watch_path)) {
                known_files.insert(entry.path().string());
            }
        }
    }

    void start_watching(int interval_seconds = 2) {
        std::cout << "Starting to watch: " << watch_path << std::endl;
        watching = true;
        watcher_thread = std::thread(&FileWatcher::watch_loop, this, interval_seconds);
    }

    void stop_watching() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            watching = false;
        }
        cv.notify_all();
        if (watcher_thread.joinable()) {
            watcher_thread.join();
        }
        std::cout << "Stopped watching." << std::endl;
    }

    ~FileWatcher() {
        if (watching) {
            stop_watching();
        }
    }

private:
    void watch_loop(int interval_seconds) {
        while (true) {
            std::unique_lock<std::mutex> lock(mtx);
            if (cv.wait_for(lock, std::chrono::seconds(interval_seconds), [this] { return !watching; })) {
                break;
            }
            lock.unlock();

            check_for_changes();
        }
    }

    void check_for_changes() {
        std::unordered_set<std::string> current_files;

        if (fs::exists(watch_path) && fs::is_directory(watch_path)) {
            for (const auto& entry : fs::directory_iterator(watch_path)) {
                current_files.insert(entry.path().string());
            }
        }

        for (const auto& file : current_files) {
            if (known_files.find(file) == known_files.end()) {
                std::cout << "[NEW] " << file << std::endl;
            }
        }

        for (const auto& file : known_files) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "[DELETED] " << file << std::endl;
            }
        }

        known_files = std::move(current_files);
    }

    fs::path watch_path;
    std::unordered_set<std::string> known_files;
    std::thread watcher_thread;
    std::mutex mtx;
    std::condition_variable cv;
    bool watching = false;
};

int main() {
    try {
        FileWatcher watcher(".");
        watcher.start_watching(3);

        std::this_thread::sleep_for(std::chrono::seconds(15));
        watcher.stop_watching();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}