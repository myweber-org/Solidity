
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

    void populate_file_set() {
        current_files.clear();
        for (const auto& entry : fs::directory_iterator(path_to_watch)) {
            current_files.insert(entry.path().filename().string());
        }
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        populate_file_set();
    }

    void start_monitoring(int interval_seconds = 2) {
        running = true;
        std::cout << "Started monitoring: " << path_to_watch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            auto previous_files = current_files;
            populate_file_set();

            // Detect new files
            for (const auto& filename : current_files) {
                if (previous_files.find(filename) == previous_files.end()) {
                    std::cout << "[NEW] " << filename << std::endl;
                }
            }

            // Detect deleted files
            for (const auto& filename : previous_files) {
                if (current_files.find(filename) == current_files.end()) {
                    std::cout << "[DELETED] " << filename << std::endl;
                }
            }
        }
    }

    void stop_monitoring() {
        running = false;
        std::cout << "Stopped monitoring." << std::endl;
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start_monitoring(3);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}