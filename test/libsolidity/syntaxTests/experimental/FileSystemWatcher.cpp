
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

    void addWatchPath(const FilePath& path) {
        if (fs::exists(path)) {
            scanPath(path);
        }
    }

    void setCallback(Callback cb) {
        callback = std::move(cb);
    }

    void startMonitoring(int intervalSeconds = 1) {
        monitoring = true;
        while (monitoring) {
            checkForChanges();
            std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
        }
    }

    void stopMonitoring() {
        monitoring = false;
    }

private:
    std::unordered_map<std::string, FileTime> fileTimestamps;
    Callback callback;
    bool monitoring = false;

    void scanPath(const FilePath& path) {
        if (fs::is_directory(path)) {
            for (const auto& entry : fs::recursive_directory_iterator(path)) {
                if (fs::is_regular_file(entry.path())) {
                    fileTimestamps[entry.path().string()] = fs::last_write_time(entry.path());
                }
            }
        } else if (fs::is_regular_file(path)) {
            fileTimestamps[path.string()] = fs::last_write_time(path);
        }
    }

    void checkForChanges() {
        auto currentTimestamps = fileTimestamps;
        
        for (const auto& [filePath, oldTime] : currentTimestamps) {
            if (!fs::exists(filePath)) {
                fileTimestamps.erase(filePath);
                if (callback) {
                    callback(filePath, "deleted");
                }
                continue;
            }

            auto currentTime = fs::last_write_time(filePath);
            if (currentTime != oldTime) {
                fileTimestamps[filePath] = currentTime;
                if (callback) {
                    callback(filePath, "modified");
                }
            }
        }

        for (const auto& entry : fs::recursive_directory_iterator(".")) {
            if (fs::is_regular_file(entry.path())) {
                std::string pathStr = entry.path().string();
                if (fileTimestamps.find(pathStr) == fileTimestamps.end()) {
                    fileTimestamps[pathStr] = fs::last_write_time(entry.path());
                    if (callback) {
                        callback(pathStr, "created");
                    }
                }
            }
        }
    }
};

int main() {
    FileSystemWatcher watcher;
    
    watcher.setCallback([](const FileSystemWatcher::FilePath& path, const std::string& action) {
        std::cout << "File: " << path << " Action: " << action << std::endl;
    });

    watcher.addWatchPath(".");
    
    std::thread monitorThread([&watcher]() {
        watcher.startMonitoring(2);
    });

    std::cout << "Monitoring started. Press Enter to stop..." << std::endl;
    std::cin.get();
    
    watcher.stopMonitoring();
    monitorThread.join();
    
    return 0;
}