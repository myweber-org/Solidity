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

    void startWatching(int interval_seconds = 2) {
        running = true;
        std::cout << "Watching directory: " << path_to_wick << std::endl;

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

    void stopWatching() {
        running = false;
    }
};

int main() {
    std::string watch_path = ".";
    FileSystemWatcher watcher(watch_path);

    std::thread watch_thread([&watcher]() {
        watcher.startWatching();
    });

    std::cout << "File system watcher started. Press Enter to stop..." << std::endl;
    std::cin.get();

    watcher.stopWatching();
    watch_thread.join();

    std::cout << "Watcher stopped." << std::endl;
    return 0;
}