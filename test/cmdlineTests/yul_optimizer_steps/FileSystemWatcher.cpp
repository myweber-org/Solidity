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

    std::unordered_set<std::string> get_directory_contents() {
        std::unordered_set<std::string> files;
        for (const auto& entry : fs::directory_iterator(path_to_watch)) {
            files.insert(entry.path().filename().string());
        }
        return files;
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        current_files = get_directory_contents();
        std::cout << "Watching directory: " << path_to_watch << std::endl;
    }

    void check_for_changes() {
        auto new_files = get_directory_contents();

        // Check for added files
        for (const auto& file : new_files) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "File added: " << file << std::endl;
            }
        }

        // Check for removed files
        for (const auto& file : current_files) {
            if (new_files.find(file) == new_files.end()) {
                std::cout << "File removed: " << file << std::endl;
            }
        }

        current_files = std::move(new_files);
    }

    void start_monitoring(int interval_seconds = 2) {
        std::cout << "Starting monitoring with interval " << interval_seconds << " seconds." << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            try {
                check_for_changes();
            } catch (const std::exception& e) {
                std::cerr << "Error checking directory: " << e.what() << std::endl;
                break;
            }
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start_monitoring(3);
    } catch (const std::exception& e) {
        std::cerr << "Failed to start watcher: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}