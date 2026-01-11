#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path watch_path;
    std::unordered_set<std::string> known_files;
    bool running;

    void scan_directory() {
        std::unordered_set<std::string> current_files;
        
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                current_files.insert(filename);
                
                if (known_files.find(filename) == known_files.end()) {
                    std::cout << "New file detected: " << filename << std::endl;
                }
            }
        }

        for (const auto& old_file : known_files) {
            if (current_files.find(old_file) == current_files.end()) {
                std::cout << "File removed: " << old_file << std::endl;
            }
        }

        known_files = std::move(current_files);
    }

public:
    FileSystemWatcher(const std::string& path) : watch_path(path), running(false) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Invalid directory path");
        }
    }

    void start_watching(int interval_seconds = 2) {
        running = true;
        std::cout << "Starting file system watcher on: " << watch_path << std::endl;
        
        scan_directory();

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            try {
                scan_directory();
            } catch (const std::exception& e) {
                std::cerr << "Error scanning directory: " << e.what() << std::endl;
            }
        }
    }

    void stop_watching() {
        running = false;
        std::cout << "File system watcher stopped." << std::endl;
    }

    void print_current_files() const {
        std::cout << "Currently tracked files:" << std::endl;
        for (const auto& file : known_files) {
            std::cout << "  - " << file << std::endl;
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start_watching(3);
    } catch (const std::exception& e) {
        std::cerr << "Failed to start watcher: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}