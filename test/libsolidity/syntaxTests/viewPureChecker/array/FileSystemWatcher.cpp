#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path path_to_watch;
    std::unordered_set<std::string> current_files;
    bool running = false;

    std::unordered_set<std::string> getDirectoryContents() {
        std::unordered_set<std::string> files;
        if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                files.insert(entry.path().filename().string());
            }
        }
        return files;
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        current_files = getDirectoryContents();
    }

    void startMonitoring(int interval_seconds = 2) {
        running = true;
        std::cout << "Starting to monitor: " << path_to_watch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            auto new_files = getDirectoryContents();

            // Check for added files
            for (const auto& file : new_files) {
                if (current_files.find(file) == current_files.end()) {
                    std::cout << "[ADDED] " << file << std::endl;
                }
            }

            // Check for removed files
            for (const auto& file : current_files) {
                if (new_files.find(file) == new_files.end()) {
                    std::cout << "[REMOVED] " << file << std::endl;
                }
            }

            current_files = new_files;
        }
    }

    void stopMonitoring() {
        running = false;
        std::cout << "Stopped monitoring." << std::endl;
    }
};

int main() {
    std::string path = ".";
    FileSystemWatcher watcher(path);

    // Monitor for 10 seconds then stop
    std::thread monitor_thread([&watcher]() {
        watcher.startMonitoring(1);
    });

    std::this_thread::sleep_for(std::chrono::seconds(10));
    watcher.stopMonitoring();
    monitor_thread.join();

    return 0;
}