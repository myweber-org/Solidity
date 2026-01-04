#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>
#include <functional>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher(const fs::path& directory, Callback callback)
        : watch_directory(directory), notify_callback(callback), running(false) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

    void start() {
        running = true;
        scan_thread = std::thread(&FileSystemWatcher::monitor, this);
    }

    void stop() {
        running = false;
        if (scan_thread.joinable()) {
            scan_thread.join();
        }
    }

private:
    void monitor() {
        std::unordered_set<std::string> previous_files;

        // Initial scan
        for (const auto& entry : fs::directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.status())) {
                previous_files.insert(entry.path().string());
            }
        }

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            std::unordered_set<std::string> current_files;
            std::vector<std::string> added_files;
            std::vector<std::string> removed_files;

            // Current scan
            for (const auto& entry : fs::directory_iterator(watch_directory)) {
                if (fs::is_regular_file(entry.status())) {
                    current_files.insert(entry.path().string());
                }
            }

            // Detect added files
            for (const auto& file : current_files) {
                if (previous_files.find(file) == previous_files.end()) {
                    added_files.push_back(file);
                }
            }

            // Detect removed files
            for (const auto& file : previous_files) {
                if (current_files.find(file) == current_files.end()) {
                    removed_files.push_back(file);
                }
            }

            // Trigger callbacks
            for (const auto& file : added_files) {
                notify_callback(file, "added");
            }
            for (const auto& file : removed_files) {
                notify_callback(file, "removed");
            }

            previous_files = std::move(current_files);
        }
    }

    fs::path watch_directory;
    Callback notify_callback;
    std::thread scan_thread;
    std::atomic<bool> running;
};

// Example usage (commented out as per instructions)
/*
int main() {
    FileSystemWatcher watcher("./test_dir", [](const fs::path& path, const std::string& action) {
        std::cout << "File " << path << " was " << action << std::endl;
    });

    watcher.start();
    std::this_thread::sleep_for(std::chrono::seconds(30));
    watcher.stop();

    return 0;
}
*/