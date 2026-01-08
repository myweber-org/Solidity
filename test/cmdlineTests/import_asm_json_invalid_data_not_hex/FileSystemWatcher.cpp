
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
    using FilePath = fs::path;
    using FileTime = fs::file_time_type;
    using Callback = std::function<void(const FilePath&, const std::string&)>;

    FileSystemWatcher() = default;

    void watchDirectory(const FilePath& directory, Callback callback) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Error: " << directory << " is not a valid directory.\n";
            return;
        }

        std::cout << "Watching directory: " << directory << std::endl;
        
        std::unordered_map<FilePath, FileTime> fileTimestamps;

        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (fs::is_regular_file(entry.path())) {
                fileTimestamps[entry.path()] = fs::last_write_time(entry.path());
            }
        }

        while (isRunning) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            for (const auto& entry : fs::recursive_directory_iterator(directory)) {
                if (!fs::is_regular_file(entry.path())) {
                    continue;
                }

                const auto& currentPath = entry.path();
                auto currentTime = fs::last_write_time(currentPath);

                if (fileTimestamps.find(currentPath) == fileTimestamps.end()) {
                    fileTimestamps[currentPath] = currentTime;
                    callback(currentPath, "CREATED");
                } else if (fileTimestamps[currentPath] != currentTime) {
                    fileTimestamps[currentPath] = currentTime;
                    callback(currentPath, "MODIFIED");
                }
            }

            auto it = fileTimestamps.begin();
            while (it != fileTimestamps.end()) {
                if (!fs::exists(it->first)) {
                    callback(it->first, "DELETED");
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

void exampleCallback(const fs::path& filePath, const std::string& changeType) {
    std::cout << "File: " << filePath.filename() << " - Action: " << changeType << std::endl;
}

int main() {
    FileSystemWatcher watcher;
    
    std::thread watchThread([&watcher]() {
        watcher.watchDirectory("./test_directory", exampleCallback);
    });

    std::this_thread::sleep_for(std::chrono::seconds(30));
    
    watcher.stop();
    watchThread.join();

    return 0;
}