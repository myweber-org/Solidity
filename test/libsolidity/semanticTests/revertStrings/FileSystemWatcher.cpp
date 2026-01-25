#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path watchPath;
    std::unordered_map<std::string, fs::file_time_type> fileTimestamps;
    bool running;

    void scanDirectory() {
        for (const auto& entry : fs::recursive_directory_iterator(watchPath)) {
            if (entry.is_regular_file()) {
                auto path = entry.path().string();
                auto currentTime = fs::last_write_time(entry);

                if (fileTimestamps.find(path) == fileTimestamps.end()) {
                    std::cout << "File added: " << path << std::endl;
                    fileTimestamps[path] = currentTime;
                } else if (fileTimestamps[path] != currentTime) {
                    std::cout << "File modified: " << path << std::endl;
                    fileTimestamps[path] = currentTime;
                }
            }
        }

        std::vector<std::string> toRemove;
        for (const auto& [path, timestamp] : fileTimestamps) {
            if (!fs::exists(path)) {
                std::cout << "File deleted: " << path << std::endl;
                toRemove.push_back(path);
            }
        }
        for (const auto& path : toRemove) {
            fileTimestamps.erase(path);
        }
    }

public:
    FileSystemWatcher(const std::string& path) : watchPath(path), running(false) {
        if (!fs::exists(watchPath) || !fs::is_directory(watchPath)) {
            throw std::runtime_error("Invalid directory path");
        }
    }

    void start(int intervalSeconds = 2) {
        running = true;
        std::cout << "Watching directory: " << watchPath.string() << std::endl;
        
        while (running) {
            scanDirectory();
            std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
        }
    }

    void stop() {
        running = false;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}