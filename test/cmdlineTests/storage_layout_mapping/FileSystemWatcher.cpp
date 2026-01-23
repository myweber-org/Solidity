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

public:
    explicit FileSystemWatcher(const std::string& path) : directory_path(path) {
        if (!fs::exists(directory_path) || !fs::is_directory(directory_path)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        initializeKnownFiles();
    }

    void initializeKnownFiles() {
        known_files.clear();
        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (fs::is_regular_file(entry.path())) {
                known_files.insert(entry.path().filename().string());
            }
        }
    }

    void watch(int interval_seconds = 2) {
        std::cout << "Watching directory: " << directory_path << std::endl;
        std::cout << "Initial file count: " << known_files.size() << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            checkForNewFiles();
        }
    }

    void checkForNewFiles() {
        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (fs::is_regular_file(entry.path())) {
                std::string filename = entry.path().filename().string();
                if (known_files.find(filename) == known_files.end()) {
                    known_files.insert(filename);
                    std::cout << "New file detected: " << filename << std::endl;
                    std::cout << "  Size: " << fs::file_size(entry.path()) << " bytes" << std::endl;
                    std::cout << "  Last modified: " << fs::last_write_time(entry.path()) << std::endl;
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
        FileSystemWatcher watcher(argv[1]);
        watcher.watch();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}