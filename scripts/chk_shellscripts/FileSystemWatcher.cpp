#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path watchPath;
    std::unordered_set<std::string> currentFiles;
    bool running;

    void scanDirectory() {
        std::unordered_set<std::string> newFiles;
        for (const auto& entry : fs::directory_iterator(watchPath)) {
            if (entry.is_regular_file()) {
                newFiles.insert(entry.path().filename().string());
            }
        }

        for (const auto& file : newFiles) {
            if (currentFiles.find(file) == currentFiles.end()) {
                std::cout << "File added: " << file << std::endl;
            }
        }

        for (const auto& file : currentFiles) {
            if (newFiles.find(file) == newFiles.end()) {
                std::cout << "File removed: " << file << std::endl;
            }
        }

        currentFiles = std::move(newFiles);
    }

public:
    FileSystemWatcher(const std::string& path) : watchPath(path), running(false) {
        if (!fs::exists(watchPath) || !fs::is_directory(watchPath)) {
            throw std::runtime_error("Invalid directory path");
        }
        scanDirectory();
    }

    void startMonitoring(int intervalSeconds = 5) {
        running = true;
        std::cout << "Monitoring directory: " << watchPath.string() << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
            try {
                scanDirectory();
            } catch (const std::exception& e) {
                std::cerr << "Error scanning directory: " << e.what() << std::endl;
            }
        }
    }

    void stopMonitoring() {
        running = false;
    }
};

int main() {
    try {
        FileSystemWatcher watcher("./logs");
        watcher.startMonitoring(3);
    } catch (const std::exception& e) {
        std::cerr << "Failed to start watcher: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}