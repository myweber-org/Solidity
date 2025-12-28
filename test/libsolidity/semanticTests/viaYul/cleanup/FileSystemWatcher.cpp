
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
        }
    }

    void setCallback(Callback cb) {
        callback_ = std::move(cb);
    }

    void startMonitoring(int intervalMs = 1000) {
        monitoring_ = true;
        while (monitoring_) {
            checkForChanges();
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
        }
    }

    void stopMonitoring() {
        monitoring_ = false;
    }

private:
    struct FileState {
        FileTime lastWriteTime;
        uintmax_t fileSize;
        bool exists;
    };

    std::unordered_map<std::string, FileState> fileStates_;
    Callback callback_;
    bool monitoring_{false};

    void updateFileState(const FilePath& path) {
        std::string key = path.string();
        FileState state;

        try {
            if (fs::exists(path)) {
                state.lastWriteTime = fs::last_write_time(path);
                state.fileSize = fs::file_size(path);
                state.exists = true;
            } else {
                state.exists = false;
            }
        } catch (const fs::filesystem_error&) {
            state.exists = false;
        }

        fileStates_[key] = state;
    }

    void checkForChanges() {
        for (auto& [pathStr, oldState] : fileStates_) {
            FilePath path(pathStr);
            FileState newState;

            try {
                if (fs::exists(path)) {
                    newState.lastWriteTime = fs::last_write_time(path);
                    newState.fileSize = fs::file_size(path);
                    newState.exists = true;
                } else {
                    newState.exists = false;
                }
            } catch (const fs::filesystem_error&) {
                newState.exists = false;
            }

            if (!oldState.exists && newState.exists) {
                notifyChange(path, "CREATED");
            } else if (oldState.exists && !newState.exists) {
                notifyChange(path, "DELETED");
            } else if (oldState.exists && newState.exists) {
                if (oldState.lastWriteTime != newState.lastWriteTime) {
                    notifyChange(path, "MODIFIED");
                } else if (oldState.fileSize != newState.fileSize) {
                    notifyChange(path, "SIZE_CHANGED");
                }
            }

            oldState = newState;
        }
    }

    void notifyChange(const FilePath& path, const std::string& changeType) {
        if (callback_) {
            callback_(path, changeType);
        }
    }
};

int main() {
    FileSystemWatcher watcher;

    watcher.setCallback([](const FileSystemWatcher::FilePath& path, const std::string& change) {
        std::cout << "File: " << path << " - Change: " << change << std::endl;
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