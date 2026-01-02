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
        TimePoint lastModified;
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
            auto ftime = fs::last_write_time(path);
            auto fsize = fs::file_size(path);
            fileMap_[path] = {ftime, fsize};
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error accessing file: " << e.what() << std::endl;
        }
    }

    void checkForChanges() {
        auto currentMap = fileMap_;
        for (const auto& [path, oldInfo] : currentMap) {
            if (!fs::exists(path)) {
                if (callback_) {
                    callback_(path, "deleted");
                }
                fileMap_.erase(path);
                continue;
            }

            try {
                auto currentTime = fs::last_write_time(path);
                auto currentSize = fs::file_size(path);

                if (currentTime != oldInfo.lastModified || currentSize != oldInfo.fileSize) {
                    fileMap_[path] = {currentTime, currentSize};
                    if (callback_) {
                        callback_(path, "modified");
                    }
                }
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error checking file: " << e.what() << std::endl;
            }
        }

        for (const auto& entry : fs::recursive_directory_iterator(getWatchedRoot())) {
            if (fs::is_regular_file(entry.status())) {
                const auto& path = entry.path();
                if (fileMap_.find(path) == fileMap_.end()) {
                    updateFileInfo(path);
                    if (callback_) {
                        callback_(path, "created");
                    }
                }
            }
        }
    }

    FilePath getWatchedRoot() const {
        if (!fileMap_.empty()) {
            return fileMap_.begin()->first.parent_path();
        }
        return FilePath();
    }
};

int main() {
    FileSystemWatcher watcher;
    
    watcher.setCallback([](const FileSystemWatcher::FilePath& path, const std::string& action) {
        std::cout << "File " << path << " was " << action << std::endl;
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