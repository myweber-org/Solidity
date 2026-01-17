
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
    using TimePoint = fs::file_time_type;
    using Callback = std::function<void(const FilePath&, const std::string&)>;

    FileSystemWatcher() = default;

    void addWatchPath(const FilePath& path) {
        if (fs::exists(path) && fs::is_directory(path)) {
            scanDirectory(path);
        }
    }

    void setCallback(Callback cb) {
        callback = std::move(cb);
    }

    void start() {
        running = true;
        watchThread = std::thread(&FileSystemWatcher::watchLoop, this);
    }

    void stop() {
        running = false;
        if (watchThread.joinable()) {
            watchThread.join();
        }
    }

private:
    std::unordered_map<FilePath, TimePoint> fileMap;
    Callback callback;
    std::thread watchThread;
    bool running = false;

    void scanDirectory(const FilePath& directory) {
        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                fileMap[entry.path()] = fs::last_write_time(entry);
            }
        }
    }

    void watchLoop() {
        while (running) {
            auto currentState = fileMap;
            for (const auto& [path, lastTime] : currentState) {
                if (!fs::exists(path)) {
                    fileMap.erase(path);
                    if (callback) {
                        callback(path, "deleted");
                    }
                    continue;
                }

                auto currentTime = fs::last_write_time(path);
                if (currentTime != lastTime) {
                    fileMap[path] = currentTime;
                    if (callback) {
                        callback(path, "modified");
                    }
                }
            }

            for (const auto& entry : fs::recursive_directory_iterator(fileMap.begin()->first.parent_path())) {
                if (fs::is_regular_file(entry.status())) {
                    FilePath currentPath = entry.path();
                    if (fileMap.find(currentPath) == fileMap.end()) {
                        fileMap[currentPath] = fs::last_write_time(entry);
                        if (callback) {
                            callback(currentPath, "created");
                        }
                    }
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
};

int main() {
    FileSystemWatcher watcher;
    watcher.addWatchPath(".");
    
    watcher.setCallback([](const FileSystemWatcher::FilePath& path, const std::string& action) {
        std::cout << "File: " << path.filename() << " Action: " << action << std::endl;
    });

    watcher.start();
    
    std::this_thread::sleep_for(std::chrono::seconds(30));
    
    watcher.stop();
    
    return 0;
}