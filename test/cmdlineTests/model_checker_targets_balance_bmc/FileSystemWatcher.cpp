#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& directory) : watch_directory(directory) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        update_snapshot();
    }

    void start_watching(int interval_seconds = 2) {
        std::cout << "Watching directory: " << watch_directory << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path watch_directory;
    std::unordered_set<std::string> previous_files;

    void update_snapshot() {
        previous_files.clear();
        for (const auto& entry : fs::directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                previous_files.insert(entry.path().filename().string());
            }
        }
    }

    void check_for_changes() {
        std::unordered_set<std::string> current_files;
        for (const auto& entry : fs::directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                current_files.insert(entry.path().filename().string());
            }
        }

        // Check for new files
        for (const auto& file : current_files) {
            if (previous_files.find(file) == previous_files.end()) {
                std::cout << "[NEW] File detected: " << file << std::endl;
            }
        }

        // Check for deleted files
        for (const auto& file : previous_files) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "[DELETED] File removed: " << file << std::endl;
            }
        }

        previous_files = std::move(current_files);
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start_watching();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}