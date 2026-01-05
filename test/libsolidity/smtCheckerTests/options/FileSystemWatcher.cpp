
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>
#include <functional>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using FileChangeCallback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher() = default;

    void watchDirectory(const fs::path& directory, FileChangeCallback callback) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Error: " << directory << " is not a valid directory." << std::endl;
            return;
        }

        std::cout << "Watching directory: " << directory << std::endl;
        std::unordered_map<std::string, fs::file_time_type> fileTimestamps;

        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.path())) {
                fileTimestamps[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }

        while (isRunning) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            for (const auto& entry : fs::directory_iterator(directory)) {
                if (!fs::is_regular_file(entry.path())) continue;

                auto currentPath = entry.path();
                auto currentTime = fs::last_write_time(currentPath);
                auto pathStr = currentPath.string();

                if (fileTimestamps.find(pathStr) == fileTimestamps.end()) {
                    std::cout << "New file detected: " << currentPath.filename() << std::endl;
                    fileTimestamps[pathStr] = currentTime;
                    if (callback) callback(currentPath, "created");
                } else if (fileTimestamps[pathStr] != currentTime) {
                    std::cout << "File modified: " << currentPath.filename() << std::endl;
                    fileTimestamps[pathStr] = currentTime;
                    if (callback) callback(currentPath, "modified");
                }
            }

            auto it = fileTimestamps.begin();
            while (it != fileTimestamps.end()) {
                if (!fs::exists(fs::path(it->first))) {
                    std::cout << "File deleted: " << fs::path(it->first).filename() << std::endl;
                    if (callback) callback(fs::path(it->first), "deleted");
                    it = fileTimestamps.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    void stop() {
        isRunning = false;
    }

private:
    bool isRunning = true;
};

int main() {
    FileSystemWatcher watcher;

    auto callback = [](const fs::path& filePath, const std::string& changeType) {
        std::cout << "Change detected - Type: " << changeType 
                  << ", File: " << filePath.filename() << std::endl;
    };

    std::thread watchThread([&watcher, callback]() {
        watcher.watchDirectory(fs::current_path(), callback);
    });

    std::cout << "File system watcher started. Press Enter to stop..." << std::endl;
    std::cin.get();

    watcher.stop();
    if (watchThread.joinable()) {
        watchThread.join();
    }

    std::cout << "File system watcher stopped." << std::endl;
    return 0;
}