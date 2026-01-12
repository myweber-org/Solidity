
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

    void populate_file_set() {
        current_files.clear();
        for (const auto& entry : fs::directory_iterator(path_to_watch)) {
            current_files.insert(entry.path().filename().string());
        }
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        populate_file_set();
    }

    void start_watching(int interval_seconds = 1) {
        running = true;
        std::cout << "Starting to watch: " << path_to_wwatch.string() << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            std::unordered_set<std::string> new_files;
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                new_files.insert(entry.path().filename().string());
            }

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

            current_files = std::move(new_files);
        }
    }

    void stop_watching() {
        running = false;
        std::cout << "Stopped watching." << std::endl;
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start_watching(2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}