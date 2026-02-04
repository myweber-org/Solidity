
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path directory_path;
    std::unordered_set<std::string> tracked_files;
    bool running;

    void initialize_tracked_files() {
        tracked_files.clear();
        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (fs::is_regular_file(entry.status())) {
                tracked_files.insert(entry.path().filename().string());
            }
        }
    }

public:
    FileSystemWatcher(const std::string& path) : directory_path(path), running(false) {
        if (!fs::exists(directory_path) || !fs::is_directory(directory_path)) {
            throw std::runtime_error("Invalid directory path provided");
        }
        initialize_tracked_files();
    }

    void start_monitoring(int interval_seconds = 2) {
        running = true;
        std::cout << "Monitoring directory: " << directory_path << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            std::unordered_set<std::string> current_files;
            for (const auto& entry : fs::directory_iterator(directory_path)) {
                if (fs::is_regular_file(entry.status())) {
                    current_files.insert(entry.path().filename().string());
                }
            }

            for (const auto& file : current_files) {
                if (tracked_files.find(file) == tracked_files.end()) {
                    std::cout << "New file detected: " << file << std::endl;
                }
            }

            for (const auto& file : tracked_files) {
                if (current_files.find(file) == current_files.end()) {
                    std::cout << "File removed: " << file << std::endl;
                }
            }

            tracked_files = std::move(current_files);
        }
    }

    void stop_monitoring() {
        running = false;
        std::cout << "Monitoring stopped." << std::endl;
    }

    void print_current_files() const {
        std::cout << "Currently tracked files:" << std::endl;
        for (const auto& file : tracked_files) {
            std::cout << "  - " << file << std::endl;
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.print_current_files();
        
        std::thread monitor_thread([&watcher]() {
            watcher.start_monitoring();
        });

        std::this_thread::sleep_for(std::chrono::seconds(10));
        watcher.stop_monitoring();
        monitor_thread.join();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}