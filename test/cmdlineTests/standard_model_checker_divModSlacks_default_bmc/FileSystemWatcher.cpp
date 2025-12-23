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
            if (fs::is_regular_file(entry.path())) {
                auto currentTime = fs::last_write_time(entry.path());
                std::string filePath = entry.path().string();

                if (fileTimestamps.find(filePath) == fileTimestamps.end()) {
                    std::cout << "[NEW] File detected: " << filePath << std::endl;
                    fileTimestamps[filePath] = currentTime;
                } else {
                    if (fileTimestamps[filePath] != currentTime) {
                        std::cout << "[MODIFIED] File changed: " << filePath << std::endl;
                        fileTimestamps[filePath] = currentTime;
                    }
                }
            }
        }

        std::vector<std::string> toRemove;
        for (const auto& [filePath, timestamp] : fileTimestamps) {
            if (!fs::exists(filePath)) {
                std::cout << "[DELETED] File removed: " << filePath << std::endl;
                toRemove.push_back(filePath);
            }
        }

        for (const auto& filePath : toRemove) {
            fileTimestamps.erase(filePath);
        }
    }

public:
    FileSystemWatcher(const std::string& path) : watchPath(path), running(false) {
        if (!fs::exists(watchPath) || !fs::is_directory(watchPath)) {
            throw std::runtime_error("Provided path does not exist or is not a directory");
        }
    }

    void start(int intervalSeconds = 2) {
        running = true;
        std::cout << "Starting file system watcher on: " << watchPath << std::endl;
        std::cout << "Polling interval: " << intervalSeconds << " seconds" << std::endl;

        while (running) {
            try {
                scanDirectory();
            } catch (const std::exception& e) {
                std::cerr << "Error scanning directory: " << e.what() << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
        }
    }

    void stop() {
        running = false;
        std::cout << "File system watcher stopped." << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start();
    } catch (const std::exception& e) {
        std::cerr << "Failed to start watcher: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}