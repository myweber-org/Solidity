
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path directory;
    std::unordered_set<std::string> knownFiles;
    bool running;

    void scanDirectory() {
        std::unordered_set<std::string> currentFiles;
        
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                currentFiles.insert(filename);
                
                if (knownFiles.find(filename) == knownFiles.end()) {
                    std::cout << "New file detected: " << filename << std::endl;
                    knownFiles.insert(filename);
                }
            }
        }

        for (auto it = knownFiles.begin(); it != knownFiles.end(); ) {
            if (currentFiles.find(*it) == currentFiles.end()) {
                std::cout << "File removed: " << *it << std::endl;
                it = knownFiles.erase(it);
            } else {
                ++it;
            }
        }
    }

public:
    FileSystemWatcher(const std::string& dir) : directory(dir), running(false) {
        if (!fs::exists(directory)) {
            throw std::runtime_error("Directory does not exist");
        }
        
        scanDirectory();
        std::cout << "Watching directory: " << directory << std::endl;
    }

    void start(int intervalSeconds = 5) {
        running = true;
        std::cout << "Starting file system watcher..." << std::endl;
        
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
            try {
                scanDirectory();
            } catch (const std::exception& e) {
                std::cerr << "Error scanning directory: " << e.what() << std::endl;
            }
        }
    }

    void stop() {
        running = false;
        std::cout << "File system watcher stopped." << std::endl;
    }
};

int main() {
    try {
        FileSystemWatcher watcher("./logs");
        watcher.start(3);
        
        std::this_thread::sleep_for(std::chrono::seconds(30));
        watcher.stop();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}