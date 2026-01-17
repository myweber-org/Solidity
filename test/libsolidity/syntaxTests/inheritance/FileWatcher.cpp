#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    SimpleFileWatcher(const fs::path& directory) : watch_dir(directory) {
        if (!fs::exists(watch_dir) || !fs::is_directory(watch_dir)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        update_file_set();
    }

    void start_watching(int interval_seconds = 2) {
        std::cout << "Watching directory: " << watch_dir << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path watch_dir;
    std::unordered_set<std::string> known_files;

    void update_file_set() {
        known_files.clear();
        for (const auto& entry : fs::directory_iterator(watch_dir)) {
            known_files.insert(entry.path().filename().string());
        }
    }

    void check_for_changes() {
        auto current_files = std::unordered_set<std::string>();
        for (const auto& entry : fs::directory_iterator(watch_dir)) {
            current_files.insert(entry.path().filename().string());
        }

        for (const auto& file : current_files) {
            if (known_files.find(file) == known_files.end()) {
                std::cout << "New file detected: " << file << std::endl;
            }
        }

        for (const auto& file : known_files) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "File removed: " << file << std::endl;
            }
        }

        known_files = std::move(current_files);
    }
};

int main() {
    try {
        SimpleFileWatcher watcher(".");
        watcher.start_watching(1);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}