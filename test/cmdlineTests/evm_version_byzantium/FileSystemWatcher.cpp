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
        for (const auto& entry : fs::directory_iterator(watchPath)) {
            if (fs::is_regular_file(entry.status())) {
                auto currentTime = fs::last_write_time(entry.path());
                std::string filename = entry.path().filename().string();

                if (fileTimestamps.find(filename) == fileTimestamps.end()) {
                    std::cout << "New file detected: " << filename << std::endl;
                    fileTimestamps[filename] = currentTime;
                } else {
                    if (fileTimestamps[filename] != currentTime) {
                        std::cout << "File modified: " << filename << std::endl;
                        fileTimestamps[filename] = currentTime;
                    }
                }
            }
        }

        std::vector<std::string> toRemove;
        for (const auto& [filename, timestamp] : fileTimestamps) {
            if (!fs::exists(watchPath / filename)) {
                std::cout << "File deleted: " << filename << std::endl;
                toRemove.push_back(filename);
            }
        }

        for (const auto& filename : toRemove) {
            fileTimestamps.erase(filename);
        }
    }

public:
    SimpleFileWatcher(const std::string& path) : watchPath(path), running(false) {
        if (!fs::exists(watchPath) || !fs::is_directory(watchPath)) {
            throw std::runtime_error("Invalid directory path");
        }
    }

    void start(int intervalSeconds = 2) {
        running = true;
        std::cout << "Starting file watcher on: " << watchPath.string() << std::endl;

        fileTimestamps.clear();
        scanDirectory();

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
            scanDirectory();
        }
    }

    void stop() {
        running = false;
        std::cout << "File watcher stopped." << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        SimpleFileWatcher watcher(argv[1]);
        watcher.start();

        std::cout << "Press Enter to stop watching..." << std::endl;
        std::cin.get();

        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}