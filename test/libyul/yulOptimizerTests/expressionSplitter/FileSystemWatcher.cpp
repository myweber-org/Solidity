#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& directory) : watch_directory(directory) {
        if (!fs::exists(watch_directory) || !fs::is_directory(watch_directory)) {
            throw std::runtime_error("Provided path is not a valid directory.");
        }
        populate_file_set();
    }

    void start_watching(int interval_seconds = 2) {
        std::cout << "Watching directory: " << watch_directory << std::endl;
        std::cout << "Press Ctrl+C to stop." << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_new_files();
        }
    }

private:
    fs::path watch_directory;
    std::unordered_set<std::string> known_files;

    void populate_file_set() {
        known_files.clear();
        for (const auto& entry : fs::directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.status())) {
                known_files.insert(entry.path().filename().string());
            }
        }
    }

    void check_for_new_files() {
        for (const auto& entry : fs::directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.status())) {
                std::string filename = entry.path().filename().string();
                if (known_files.find(filename) == known_files.end()) {
                    std::cout << "New file detected: " << filename << std::endl;
                    known_files.insert(filename);
                }
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
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