#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class SimpleFileWatcher {
private:
    fs::path watchPath;
    std::unordered_map<std::string, fs::file_time_type> fileTimestamps;
    bool running;

    void scanDirectory() {
        for (const auto& entry : fs::recursive_directory_iterator(watchPath)) {
            if (entry.is_regular_file()) {
                auto path = entry.path();
                auto currentTime = fs::last_write_time(path);

                std::string pathStr = path.string();
                auto it = fileTimestamps.find(pathStr);

                if (it == fileTimestamps.end()) {
                    std::cout << "[CREATED] " << pathStr << std::endl;
                    fileTimestamps[pathStr] = currentTime;
                } else if (it->second != currentTime) {
                    std::cout << "[MODIFIED] " << pathStr << std::endl;
                    fileTimestamps[pathStr] = currentTime;
                }
            }
        }

        std::vector<std::string> toRemove;
        for (const auto& [path, timestamp] : fileTimestamps) {
            if (!fs::exists(path)) {
                std::cout << "[DELETED] " << path << std::endl;
                toRemove.push_back(path);
            }
        }

        for (const auto& path : toRemove) {
            fileTimestamps.erase(path);
        }
    }

public:
    SimpleFileWatcher(const std::string& path) : watchPath(path), running(false) {
        if (!fs::exists(watchPath) || !fs::is_directory(watchPath)) {
            throw std::runtime_error("Path does not exist or is not a directory");
        }
    }

    void start(int intervalSeconds = 2) {
        running = true;
        std::cout << "Watching directory: " << watchPath.string() << std::endl;
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
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        SimpleFileWatcher watcher(argv[1]);
        watcher.start();
    } catch (const std::exception& e) {
        std::cerr << "Failed to start watcher: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}