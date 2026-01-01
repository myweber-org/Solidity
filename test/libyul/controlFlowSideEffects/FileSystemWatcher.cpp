
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
        try {
            FileState state;
            state.exists = fs::exists(path);
            if (state.exists) {
                state.lastWriteTime = fs::last_write_time(path);
                state.fileSize = fs::file_size(path);
            }
            fileStates_[path.string()] = state;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << '\n';
        }
    }

    void checkForChanges() {
        for (auto& [pathStr, oldState] : fileStates_) {
            FilePath path(pathStr);
            try {
                bool currentExists = fs::exists(path);
                
                if (!oldState.exists && currentExists) {
                    notify(path, "CREATED");
                    updateFileState(path);
                } else if (oldState.exists && !currentExists) {
                    notify(path, "DELETED");
                    updateFileState(path);
                } else if (currentExists) {
                    auto currentWriteTime = fs::last_write_time(path);
                    auto currentSize = fs::file_size(path);
                    
                    if (currentWriteTime != oldState.lastWriteTime) {
                        notify(path, "MODIFIED");
                        updateFileState(path);
                    } else if (currentSize != oldState.fileSize) {
                        notify(path, "SIZE_CHANGED");
                        updateFileState(path);
                    }
                }
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error checking path " << pathStr << ": " << e.what() << '\n';
            }
        }
    }

    void notify(const FilePath& path, const std::string& event) {
        if (callback_) {
            callback_(path, event);
        }
    }
};

int main() {
    FileSystemWatcher watcher;
    
    watcher.setCallback([](const FileSystemWatcher::FilePath& path, const std::string& event) {
        std::cout << "Event: " << event << " | Path: " << path << std::endl;
    });

    watcher.addWatchPath("test_directory");
    watcher.addWatchPath("example.txt");

    std::cout << "Starting file system watcher. Monitoring for changes...\n";
    std::cout << "Press Ctrl+C to stop.\n";

    watcher.startMonitoring(2000);

    return 0;
}