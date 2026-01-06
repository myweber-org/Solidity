
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path directory_to_watch;
    std::unordered_set<std::string> known_files;
    bool running;

    std::unordered_set<std::string> getCurrentFileList() const {
        std::unordered_set<std::string> current_files;
        try {
            for (const auto& entry : fs::directory_iterator(directory_to_watch)) {
                if (entry.is_regular_file()) {
                    current_files.insert(entry.path().filename().string());
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
        return current_files;
    }

    void compareAndNotify(const std::unordered_set<std::string>& current_files) {
        // Check for new files
        for (const auto& file : current_files) {
            if (known_files.find(file) == known_files.end()) {
                std::cout << "[NEW] File detected: " << file << std::endl;
            }
        }

        // Check for deleted files
        for (const auto& file : known_files) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "[DELETED] File removed: " << file << std::endl;
            }
        }
    }

public:
    explicit FileSystemWatcher(const std::string& dir_path) 
        : directory_to_watch(dir_path), running(false) {
        if (!fs::exists(directory_to_watch) || !fs::is_directory(directory_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        known_files = getCurrentFileList();
        std::cout << "Watching directory: " << fs::absolute(directory_to_watch) << std::endl;
        std::cout << "Initial file count: " << known_files.size() << std::endl;
    }

    void start(int interval_seconds = 2) {
        running = true;
        std::cout << "Starting watcher with interval " << interval_seconds << " seconds." << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            
            auto current_files = getCurrentFileList();
            compareAndNotify(current_files);
            
            known_files = std::move(current_files);
        }
    }

    void stop() {
        running = false;
        std::cout << "Watcher stopped." << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        
        // Handle Ctrl+C or other termination signals in a simple way
        std::cout << "Press Enter to stop watching..." << std::endl;
        
        // Run watcher in a separate thread to allow main thread to wait for input
        std::thread watch_thread([&watcher]() {
            watcher.start();
        });

        std::cin.get(); // Wait for user input
        watcher.stop();
        watch_thread.join();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}