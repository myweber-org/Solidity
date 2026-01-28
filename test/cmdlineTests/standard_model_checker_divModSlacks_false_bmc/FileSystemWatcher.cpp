
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
        callback_ = std::move(cb);
    }

    void startMonitoring(int intervalSeconds = 1) {
        monitoring_ = true;
        while (monitoring_) {
            checkForChanges();
            std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
        }
    }

    void stopMonitoring() {
        monitoring_ = false;
    }

private:
    struct FileInfo {
        FileTime lastWriteTime;
        uintmax_t fileSize;
    };

    std::unordered_map<FilePath, FileInfo> fileMap_;
    Callback callback_;
    bool monitoring_{false};

    void scanPath(const FilePath& path) {
        if (fs::is_directory(path)) {
            for (const auto& entry : fs::recursive_directory_iterator(path)) {
                if (fs::is_regular_file(entry.status())) {
                    updateFileInfo(entry.path());
                }
            }
        } else if (fs::is_regular_file(path)) {
            updateFileInfo(path);
        }
    }

    void updateFileInfo(const FilePath& path) {
        try {
            FileInfo info;
            info.lastWriteTime = fs::last_write_time(path);
            info.fileSize = fs::file_size(path);
            fileMap_[path] = info;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error accessing file: " << e.what() << std::endl;
        }
    }

    void checkForChanges() {
        auto currentMap = fileMap_;
        for (const auto& [path, oldInfo] : currentMap) {
            if (!fs::exists(path)) {
                fileMap_.erase(path);
                if (callback_) {
                    callback_(path, "deleted");
                }
                continue;
            }

            try {
                FileTime currentTime = fs::last_write_time(path);
                uintmax_t currentSize = fs::file_size(path);

                if (currentTime != oldInfo.lastWriteTime || currentSize != oldInfo.fileSize) {
                    FileInfo newInfo{currentTime, currentSize};
                    fileMap_[path] = newInfo;
                    if (callback_) {
                        callback_(path, "modified");
                    }
                }
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error checking file: " << e.what() << std::endl;
            }
        }

        for (const auto& entry : fs::recursive_directory_iterator(".")) {
            if (fs::is_regular_file(entry.status())) {
                FilePath path = entry.path();
                if (fileMap_.find(path) == fileMap_.end()) {
                    updateFileInfo(path);
                    if (callback_) {
                        callback_(path, "created");
                    }
                }
            }
        }
    }
};

int main() {
    FileSystemWatcher watcher;
    
    watcher.setCallback([](const FileSystemWatcher::FilePath& path, const std::string& action) {
        std::cout << "File " << path << " has been " << action << std::endl;
    });

    watcher.addWatchPath(".");
    
    std::cout << "Starting file system watcher. Monitoring current directory..." << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;

    std::thread monitorThread([&watcher]() {
        watcher.startMonitoring(2);
    });

    monitorThread.join();
    
    return 0;
}
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

    void startWatching(int intervalMs = 1000) {
        watching = true;
        while (watching) {
            checkForChanges();
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
        }
    }

    void stopWatching() {
        watching = false;
    }

private:
    std::unordered_map<FilePath, FileTime> fileStates;
    std::unordered_set<FilePath> watchPaths;
    Callback callback;
    bool watching = false;

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

    std::cout << "Starting file system watcher. Press Ctrl+C to stop." << std::endl;

    std::thread watchThread([&watcher]() {
        watcher.startWatching(500);
    });

    std::this_thread::sleep_for(std::chrono::seconds(30));

    watcher.stopWatching();
    if (watchThread.joinable()) {
        watchThread.join();
    }

    std::cout << "File system watcher stopped." << std::endl;
    return 0;
}