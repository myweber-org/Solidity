
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

    FileSystemWatcher(const fs::path& directory_path) 
        : watch_path(directory_path), running(false) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        initializeSnapshot();
    }

    void setCallback(FileChangeCallback callback) {
        changeCallback = std::move(callback);
    }

    void startWatching(int interval_seconds = 1) {
        running = true;
        watcherThread = std::thread([this, interval_seconds]() {
            while (running) {
                std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
                detectChanges();
            }
        });
    }

    void stopWatching() {
        running = false;
        if (watcherThread.joinable()) {
            watcherThread.join();
        }
    }

    ~FileSystemWatcher() {
        stopWatching();
    }

private:
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> fileSnapshot;
    FileChangeCallback changeCallback;
    std::thread watcherThread;
    bool running;

    void initializeSnapshot() {
        fileSnapshot.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.status())) {
                fileSnapshot[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void detectChanges() {
        std::unordered_map<std::string, fs::file_time_type> currentSnapshot;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.status())) {
                std::string filePath = entry.path().string();
                auto writeTime = fs::last_write_time(entry);
                currentSnapshot[filePath] = writeTime;

                auto it = fileSnapshot.find(filePath);
                if (it == fileSnapshot.end()) {
                    notifyChange(filePath, "CREATED");
                } else if (it->second != writeTime) {
                    notifyChange(filePath, "MODIFIED");
                }
            }
        }

        for (const auto& [filePath, _] : fileSnapshot) {
            if (currentSnapshot.find(filePath) == currentSnapshot.end()) {
                notifyChange(filePath, "DELETED");
            }
        }

        fileSnapshot.swap(currentSnapshot);
    }

    void notifyChange(const std::string& filePath, const std::string& changeType) {
        if (changeCallback) {
            changeCallback(filePath, changeType);
        } else {
            std::cout << "[" << changeType << "] " << filePath << std::endl;
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        
        watcher.setCallback([](const fs::path& path, const std::string& changeType) {
            std::cout << "Change detected: " << changeType << " - " << path.filename() << std::endl;
        });

        std::cout << "Starting file system watcher. Monitoring current directory..." << std::endl;
        std::cout << "Press Enter to stop watching..." << std::endl;

        watcher.startWatching();

        std::cin.get();
        watcher.stopWatching();

        std::cout << "File system watcher stopped." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}