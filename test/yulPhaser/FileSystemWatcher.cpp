#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& directory) : watch_path(directory) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Provided path is not a valid directory.");
        }
        populate_file_set();
    }

    void start_monitoring(int interval_seconds = 2) {
        std::cout << "Starting to monitor: " << watch_path.string() << std::endl;
        std::cout << "Checking every " << interval_seconds << " seconds." << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_new_files();
        }
    }

private:
    fs::path watch_path;
    std::unordered_set<std::string> known_files;

    void populate_file_set() {
        known_files.clear();
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                known_files.insert(entry.path().filename().string());
            }
        }
    }

    void check_for_new_files() {
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (known_files.find(filename) == known_files.end()) {
                    std::cout << "New file detected: " << filename << std::endl;
                    known_files.insert(filename);
                }
            }
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher("./watch_directory");
        watcher.start_monitoring();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}