
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>
#include <string>

namespace fs = std::filesystem;

class DirectoryWatcher {
public:
    explicit DirectoryWatcher(const std::string& path) : watch_path(path) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Invalid directory path");
        }
        populate_existing_files();
    }

    void start_watching(int interval_seconds = 2) {
        std::cout << "Watching directory: " << watch_path << std::endl;
        std::cout << "Press Ctrl+C to stop..." << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_new_files();
        }
    }

private:
    fs::path watch_path;
    std::unordered_set<std::string> known_files;

    void populate_existing_files() {
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                known_files.insert(entry.path().filename().string());
            }
        }
    }

    void check_for_new_files() {
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
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
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        DirectoryWatcher watcher(argv[1]);
        watcher.start_watching();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}