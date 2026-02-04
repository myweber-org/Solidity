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

    std::unordered_set<std::string> get_directory_contents() {
        std::unordered_set<std::string> files;
        if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                files.insert(entry.path().filename().string());
            }
        }
        return files;
    }

    void compare_and_notify(const std::unordered_set<std::string>& new_files) {
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
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        current_files = get_directory_contents();
    }

    void start(int interval_seconds = 2) {
        running = true;
        std::cout << "Watching directory: " << fs::absolute(path_to_watch) << std::endl;
        
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            
            auto new_files = get_directory_contents();
            compare_and_notify(new_files);
            
            current_files = std::move(new_files);
        }
    }

    void stop() {
        running = false;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}