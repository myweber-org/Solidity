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

    std::unordered_set<std::string> get_files_in_directory() {
        std::unordered_set<std::string> files;
        for (const auto& entry : fs::directory_iterator(path_to_watch)) {
            if (entry.is_regular_file()) {
                files.insert(entry.path().filename().string());
            }
        }
        return files;
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        current_files = get_files_in_directory();
        std::cout << "Watching directory: " << path_to_watch << std::endl;
    }

    void start_monitoring(int interval_seconds = 2) {
        std::cout << "Monitoring started. Checking every " << interval_seconds << " seconds." << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            auto new_files = get_files_in_directory();

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

            current_files = new_files;
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start_monitoring();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}