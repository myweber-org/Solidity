
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
            updateFileState(path);
            watchPaths.insert(path);
        }
    }

    void setCallback(Callback cb) {
        callback = std::move(cb);
    }

    void startMonitoring(int intervalMs = 1000) {
        monitoring = true;
        while (monitoring) {
            checkForChanges();
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
        }
    }

    void stopMonitoring() {
        monitoring = false;
    }

private:
    std::unordered_map<FilePath, FileTime> fileStates;
    std::unordered_set<FilePath> watchPaths;
    Callback callback;
    bool monitoring = false;

    void updateFileState(const FilePath& path) {
        if (fs::exists(path)) {
            fileStates[path] = fs::last_write_time(path);
        }
    }

    void checkForChanges() {
        for (const auto& path : watchPaths) {
            if (!fs::exists(path)) {
                if (fileStates.find(path) != fileStates.end()) {
                    fileStates.erase(path);
                    if (callback) {
                        callback(path, "deleted");
                    }
                }
                continue;
            }

            auto currentTime = fs::last_write_time(path);
            auto it = fileStates.find(path);

            if (it == fileStates.end()) {
                fileStates[path] = currentTime;
                if (callback) {
                    callback(path, "created");
                }
            } else if (it->second != currentTime) {
                it->second = currentTime;
                if (callback) {
                    callback(path, "modified");
                }
            }
        }
    }
};

int main() {
    FileSystemWatcher watcher;

    watcher.setCallback([](const FileSystemWatcher::FilePath& path, const std::string& action) {
        std::cout << "File: " << path.string() << " - Action: " << action << std::endl;
    });

    watcher.addWatchPath("test_directory");
    watcher.addWatchPath("example.txt");

    std::cout << "Starting file system watcher. Monitoring for changes..." << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;

    std::thread monitorThread([&watcher]() {
        watcher.startMonitoring(500);
    });

    monitorThread.join();

    return 0;
}