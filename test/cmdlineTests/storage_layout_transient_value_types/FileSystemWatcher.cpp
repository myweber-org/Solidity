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

    void compareAndNotify(const std::unordered_set<std::string>& new_files) {
        // Detect added files
        for (const auto& file : new_files) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "[ADDED] " << file << std::endl;
            }
        }
        // Detect removed files
        for (const auto& file : current_files) {
            if (new_files.find(file) == new_files.end()) {
                std::cout << "[REMOVED] " << file << std::endl;
            }
        }
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        current_files = getDirectoryContents();
    }

    void start(int interval_seconds = 2) {
        running = true;
        std::cout << "Watching directory: " << fs::absolute(path_to_watch) << std::endl;
        std::cout << "Press Ctrl+C to stop." << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            auto new_files = getDirectoryContents();
            compareAndNotify(new_files);
            current_files = new_files;
        }
    }

    void stop() {
        running = false;
    }
};

int main(int argc, char* argv[]) {
    std::string path = ".";
    if (argc > 1) {
        path = argv[1];
    }

    try {
        FileSystemWatcher watcher(path);
        watcher.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}