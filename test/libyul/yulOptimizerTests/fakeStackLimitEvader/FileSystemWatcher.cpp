
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
            watchPaths[path] = fs::last_write_time(path);
        }
    }

    void setCallback(Callback cb) {
        callback = std::move(cb);
    }

    void startMonitoring(int intervalSeconds = 1) {
        monitoring = true;
        monitorThread = std::thread([this, intervalSeconds]() {
            while (monitoring) {
                checkForChanges();
                std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
            }
        });
    }

    void stopMonitoring() {
        monitoring = false;
        if (monitorThread.joinable()) {
            monitorThread.join();
        }
    }

    ~FileSystemWatcher() {
        stopMonitoring();
    }

private:
    std::unordered_map<FilePath, FileTime> watchPaths;
    Callback callback;
    std::thread monitorThread;
    bool monitoring = false;

    void checkForChanges() {
        for (auto& [path, lastTime] : watchPaths) {
            if (!fs::exists(path)) {
                if (callback) {
                    callback(path, "deleted");
                }
                watchPaths.erase(path);
                continue;
            }

            auto currentTime = fs::last_write_time(path);
            if (currentTime != lastTime) {
                lastTime = currentTime;
                if (callback) {
                    callback(path, "modified");
                }
            }
        }

        for (const auto& entry : fs::recursive_directory_iterator(".")) {
            if (entry.is_regular_file()) {
                FilePath path = entry.path();
                if (watchPaths.find(path) == watchPaths.end()) {
                    watchPaths[path] = fs::last_write_time(path);
                    if (callback) {
                        callback(path, "created");
                    }
                }
            }
        }
    }
};

int main() {
    FileSystemWatcher watcher;

    watcher.setCallback([](const FileSystemWatcher::FilePath& path, const std::string& action) {
        std::cout << "File: " << path.string() << " Action: " << action << std::endl;
    });

    watcher.addWatchPath(".");
    watcher.startMonitoring(2);

    std::cout << "Monitoring started. Press Enter to stop..." << std::endl;
    std::cin.get();

    watcher.stopMonitoring();
    return 0;
}