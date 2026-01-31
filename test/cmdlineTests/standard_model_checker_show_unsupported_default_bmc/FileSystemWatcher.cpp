
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
            watchPaths.push_back(path);
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
    struct FileState {
        FileTime lastWriteTime;
        uintmax_t fileSize;
        bool exists;
    };

    std::vector<FilePath> watchPaths;
    std::unordered_map<std::string, FileState> fileStates;
    Callback callback;
    bool monitoring = false;

    void updateFileState(const FilePath& path) {
        std::string key = path.string();
        FileState state;

        if (fs::exists(path)) {
            state.lastWriteTime = fs::last_write_time(path);
            state.fileSize = fs::file_size(path);
            state.exists = true;
        } else {
            state.exists = false;
        }

        fileStates[key] = state;
    }

    void checkForChanges() {
        for (const auto& watchPath : watchPaths) {
            if (fs::is_directory(watchPath)) {
                checkDirectory(watchPath);
            } else {
                checkFile(watchPath);
            }
        }
    }

    void checkDirectory(const FilePath& dirPath) {
        try {
            for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
                if (entry.is_regular_file()) {
                    checkFile(entry.path());
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
    }

    void checkFile(const FilePath& filePath) {
        std::string key = filePath.string();
        auto it = fileStates.find(key);

        bool currentExists = fs::exists(filePath);
        FileState newState;

        if (currentExists) {
            newState.lastWriteTime = fs::last_write_time(filePath);
            newState.fileSize = fs::file_size(filePath);
            newState.exists = true;
        } else {
            newState.exists = false;
        }

        if (it == fileStates.end()) {
            if (currentExists && callback) {
                callback(filePath, "CREATED");
            }
        } else {
            const FileState& oldState = it->second;

            if (!oldState.exists && currentExists) {
                if (callback) callback(filePath, "CREATED");
            } else if (oldState.exists && !currentExists) {
                if (callback) callback(filePath, "DELETED");
            } else if (oldState.exists && currentExists) {
                if (oldState.lastWriteTime != newState.lastWriteTime ||
                    oldState.fileSize != newState.fileSize) {
                    if (callback) callback(filePath, "MODIFIED");
                }
            }
        }

        fileStates[key] = newState;
    }
};

int main() {
    FileSystemWatcher watcher;

    watcher.setCallback([](const FileSystemWatcher::FilePath& path, const std::string& action) {
        std::cout << "File: " << path << " Action: " << action << std::endl;
    });

    watcher.addWatchPath(".");
    
    std::thread monitorThread([&watcher]() {
        watcher.startMonitoring(500);
    });

    std::cout << "Monitoring started. Press Enter to stop..." << std::endl;
    std::cin.get();

    watcher.stopMonitoring();
    if (monitorThread.joinable()) {
        monitorThread.join();
    }

    return 0;
}