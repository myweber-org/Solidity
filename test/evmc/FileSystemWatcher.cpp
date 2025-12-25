
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
    using FileTime = fs::file_time_type;
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher() = default;

    void addWatchPath(const fs::path& path) {
        if (fs::exists(path) && fs::is_directory(path)) {
            scanDirectory(path);
            watchPaths.push_back(path);
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
    struct FileInfo {
        FileTime lastWriteTime;
        uintmax_t fileSize;
    };

    std::vector<fs::path> watchPaths;
    std::unordered_map<std::string, FileInfo> fileCache;
    Callback callback;
    std::thread monitorThread;
    bool monitoring = false;

    void scanDirectory(const fs::path& directory) {
        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                std::string key = entry.path().string();
                fileCache[key] = {
                    fs::last_write_time(entry),
                    fs::file_size(entry)
                };
            }
        }
    }

    void checkForChanges() {
        for (const auto& watchPath : watchPaths) {
            for (const auto& entry : fs::recursive_directory_iterator(watchPath)) {
                if (!fs::is_regular_file(entry.status())) continue;

                std::string key = entry.path().string();
                auto currentTime = fs::last_write_time(entry);
                auto currentSize = fs::file_size(entry);

                auto it = fileCache.find(key);
                if (it == fileCache.end()) {
                    fileCache[key] = {currentTime, currentSize};
                    if (callback) callback(entry.path(), "CREATED");
                } else {
                    auto& cachedInfo = it->second;
                    if (cachedInfo.lastWriteTime != currentTime || cachedInfo.fileSize != currentSize) {
                        cachedInfo = {currentTime, currentSize};
                        if (callback) callback(entry.path(), "MODIFIED");
                    }
                }
            }

            checkForDeletions(watchPath);
        }
    }

    void checkForDeletions(const fs::path& directory) {
        auto it = fileCache.begin();
        while (it != fileCache.end()) {
            if (!fs::exists(it->first)) {
                if (callback) callback(fs::path(it->first), "DELETED");
                it = fileCache.erase(it);
            } else {
                ++it;
            }
        }
    }
};

int main() {
    FileSystemWatcher watcher;
    
    watcher.setCallback([](const fs::path& path, const std::string& action) {
        std::cout << "[" << action << "] " << path.filename().string() << std::endl;
    });

    watcher.addWatchPath(".");
    watcher.startMonitoring(2);

    std::cout << "Monitoring current directory. Press Enter to stop..." << std::endl;
    std::cin.get();
    
    watcher.stopMonitoring();
    return 0;
}