#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path directory_path;
    std::unordered_set<std::string> known_files;
    bool running;

    std::unordered_set<std::string> get_current_files() {
        std::unordered_set<std::string> current;
        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (fs::is_regular_file(entry.path())) {
                current.insert(entry.path().filename().string());
            }
        }
        return current;
    }

public:
    FileSystemWatcher(const std::string& path) : directory_path(path), running(false) {
        if (!fs::exists(directory_path) || !fs::is_directory(directory_path)) {
            throw std::invalid_argument("Provided path is not a valid directory");
        }
        known_files = get_current_files();
    }

    void start_monitoring(int interval_seconds = 2) {
        running = true;
        std::cout << "Monitoring directory: " << directory_path << std::endl;
        
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            
            auto current_files = get_current_files();
            
            // Detect new files
            for (const auto& file : current_files) {
                if (known_files.find(file) == known_files.end()) {
                    std::cout << "[NEW] File detected: " << file << std::endl;
                }
            }
            
            // Detect deleted files
            for (const auto& file : known_files) {
                if (current_files.find(file) == current_files.end()) {
                    std::cout << "[DELETED] File removed: " << file << std::endl;
                }
            }
            
            known_files = current_files;
        }
    }

    void stop_monitoring() {
        running = false;
        std::cout << "Monitoring stopped." << std::endl;
    }

    void print_current_state() {
        std::cout << "Currently tracked files (" << known_files.size() << "):" << std::endl;
        for (const auto& file : known_files) {
            std::cout << "  - " << file << std::endl;
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.print_current_state();
        
        std::thread monitor_thread([&watcher]() {
            watcher.start_monitoring();
        });
        
        std::cout << "Press Enter to stop monitoring..." << std::endl;
        std::cin.get();
        
        watcher.stop_monitoring();
        monitor_thread.join();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}