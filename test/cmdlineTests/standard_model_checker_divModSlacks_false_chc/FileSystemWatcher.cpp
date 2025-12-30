
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
            watchPaths_.push_back(path);
            scanDirectory(path);
        }
    }

    void setCallback(Callback cb) {
        callback_ = std::move(cb);
    }

    void start() {
        running_ = true;
        while (running_) {
            checkForChanges();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void stop() {
        running_ = false;
    }

private:
    struct FileInfo {
        FileTime lastWriteTime;
        uintmax_t fileSize;
    };

    void scanDirectory(const fs::path& dir) {
        for (const auto& entry : fs::recursive_directory_iterator(dir)) {
            if (fs::is_regular_file(entry.status())) {
                fileMap_[entry.path()] = {
                    fs::last_write_time(entry),
                    fs::file_size(entry)
                };
            }
        }
    }

    void checkForChanges() {
        for (const auto& watchPath : watchPaths_) {
            if (!fs::exists(watchPath)) continue;

            for (const auto& entry : fs::recursive_directory_iterator(watchPath)) {
                if (!fs::is_regular_file(entry.status())) continue;

                const auto& path = entry.path();
                auto currentTime = fs::last_write_time(entry);
                auto currentSize = fs::file_size(entry);

                auto it = fileMap_.find(path);
                if (it == fileMap_.end()) {
                    fileMap_[path] = {currentTime, currentSize};
                    if (callback_) callback_(path, "CREATED");
                } else {
                    auto& storedInfo = it->second;
                    if (storedInfo.lastWriteTime != currentTime || storedInfo.fileSize != currentSize) {
                        storedInfo = {currentTime, currentSize};
                        if (callback_) callback_(path, "MODIFIED");
                    }
                }
            }

            checkForDeletions(watchPath);
        }
    }

    void checkForDeletions(const fs::path& dir) {
        auto it = fileMap_.begin();
        while (it != fileMap_.end()) {
            if (!fs::exists(it->first)) {
                if (callback_) callback_(it->first, "DELETED");
                it = fileMap_.erase(it);
            } else {
                ++it;
            }
        }
    }

    std::vector<fs::path> watchPaths_;
    std::unordered_map<fs::path, FileInfo> fileMap_;
    Callback callback_;
    bool running_{false};
};

int main() {
    FileSystemWatcher watcher;

    watcher.setCallback([](const fs::path& path, const std::string& action) {
        std::cout << "File: " << path.string() << " Action: " << action << std::endl;
    });

    watcher.addWatchPath(".");
    
    std::thread watchThread([&watcher]() {
        watcher.start();
    });

    std::cout << "Watching current directory. Press Enter to stop..." << std::endl;
    std::cin.get();

    watcher.stop();
    watchThread.join();

    return 0;
}