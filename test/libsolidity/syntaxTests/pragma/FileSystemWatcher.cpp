#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path directoryPath;
    std::unordered_set<std::string> currentFiles;

    void refreshFileList() {
        currentFiles.clear();
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (entry.is_regular_file()) {
                currentFiles.insert(entry.path().filename().string());
            }
        }
    }

public:
    explicit FileSystemWatcher(const std::string& path) : directoryPath(path) {
        if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        refreshFileList();
        std::cout << "Watching directory: " << fs::absolute(directoryPath) << std::endl;
    }

    void startMonitoring(int intervalSeconds = 2) {
        std::cout << "Starting monitoring with interval " << intervalSeconds << " seconds." << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
            checkForChanges();
        }
    }

    void checkForChanges() {
        auto previousFiles = currentFiles;
        refreshFileList();

        for (const auto& file : currentFiles) {
            if (previousFiles.find(file) == previousFiles.end()) {
                std::cout << "[ADDED] " << file << std::endl;
            }
        }

        for (const auto& file : previousFiles) {
            if (currentFiles.find(file) == currentFiles.end()) {
                std::cout << "[REMOVED] " << file << std::endl;
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
        watcher.startMonitoring();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}