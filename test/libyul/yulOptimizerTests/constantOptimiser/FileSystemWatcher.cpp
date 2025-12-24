#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class DirectoryWatcher {
private:
    fs::path watchPath;
    std::unordered_set<std::string> previousFiles;

    std::unordered_set<std::string> getCurrentFiles() {
        std::unordered_set<std::string> currentFiles;
        if (fs::exists(watchPath) && fs::is_directory(watchPath)) {
            for (const auto& entry : fs::directory_iterator(watchPath)) {
                if (entry.is_regular_file()) {
                    currentFiles.insert(entry.path().filename().string());
                }
            }
        }
        return currentFiles;
    }

public:
    DirectoryWatcher(const std::string& path) : watchPath(path) {
        if (fs::exists(watchPath) && fs::is_directory(watchPath)) {
            previousFiles = getCurrentFiles();
            std::cout << "Watching directory: " << watchPath.string() << std::endl;
        } else {
            throw std::runtime_error("Invalid directory path");
        }
    }

    void checkChanges() {
        auto currentFiles = getCurrentFiles();
        
        for (const auto& file : currentFiles) {
            if (previousFiles.find(file) == previousFiles.end()) {
                std::cout << "New file detected: " << file << std::endl;
            }
        }
        
        for (const auto& file : previousFiles) {
            if (currentFiles.find(file) == currentFiles.end()) {
                std::cout << "File removed: " << file << std::endl;
            }
        }
        
        previousFiles = currentFiles;
    }
    
    void startMonitoring(int intervalSeconds = 2) {
        std::cout << "Starting monitoring with " << intervalSeconds << " second interval" << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
            checkChanges();
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
        watcher.startMonitoring();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}