
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
            if (fs::is_regular_file(entry.status())) {
                std::string filename = entry.path().filename().string();
                current_files.insert(filename);
                
                if (known_files.find(filename) == known_files.end()) {
                    std::cout << "File added: " << filename << std::endl;
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

    void start() {
        running = true;
        std::cout << "Starting file system watcher for: " << watch_path.string() << std::endl;
        
        scan_directory();

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            scan_directory();
        }
    }

    void stop() {
        running = false;
        std::cout << "File system watcher stopped" << std::endl;
    }

    ~FileSystemWatcher() {
        if (running) {
            stop();
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}