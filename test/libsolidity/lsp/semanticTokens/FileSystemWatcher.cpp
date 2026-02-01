#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <mutex>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const fs::path& path) : watchPath(path), running(false) {}

    void start() {
        if (running) return;
        running = true;
        snapshotCurrentState();
        watcherThread = std::thread(&FileSystemWatcher::watchLoop, this);
    }

    void stop() {
        running = false;
        if (watcherThread.joinable()) {
            watcherThread.join();
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

private:
    fs::path watchPath;
    std::unordered_map<std::string, fs::file_time_type> fileTimestamps;
    std::mutex mtx;
    std::thread watcherThread;
    bool running;

    void snapshotCurrentState() {
        std::lock_guard<std::mutex> lock(mtx);
        fileTimestamps.clear();
        if (fs::exists(watchPath) && fs::is_directory(watchPath)) {
            for (const auto& entry : fs::recursive_directory_iterator(watchPath)) {
                if (fs::is_regular_file(entry.path())) {
                    fileTimestamps[entry.path().string()] = fs::last_write_time(entry.path());
                }
            }
        }
    }

    void watchLoop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            checkForChanges();
        }
    }

    void checkForChanges() {
        std::lock_guard<std::mutex> lock(mtx);
        std::unordered_map<std::string, fs::file_time_type> currentState;

        if (fs::exists(watchPath) && fs::is_directory(watchPath)) {
            for (const auto& entry : fs::recursive_directory_iterator(watchPath)) {
                if (fs::is_regular_file(entry.path())) {
                    std::string pathStr = entry.path().string();
                    currentState[pathStr] = fs::last_write_time(entry.path());
                }
            }
        }

        for (const auto& [path, timestamp] : currentState) {
            if (fileTimestamps.find(path) == fileTimestamps.end()) {
                std::cout << "File created: " << path << std::endl;
            } else if (fileTimestamps[path] != timestamp) {
                std::cout << "File modified: " << path << std::endl;
            }
        }

        for (const auto& [path, timestamp] : fileTimestamps) {
            if (currentState.find(path) == currentState.end()) {
                std::cout << "File deleted: " << path << std::endl;
            }
        }

        fileTimestamps = std::move(currentState);
    }
};

int main() {
    fs::path pathToWatch = ".";
    FileSystemWatcher watcher(pathToWatch);
    
    std::cout << "Starting file system watcher on: " << fs::absolute(pathToWatch) << std::endl;
    std::cout << "Press Enter to stop..." << std::endl;
    
    watcher.start();
    std::cin.get();
    watcher.stop();
    
    return 0;
}