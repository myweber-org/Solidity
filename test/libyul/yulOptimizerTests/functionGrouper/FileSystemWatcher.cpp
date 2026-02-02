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
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher() = default;

    void addWatchPath(const fs::path& path) {
        if (fs::exists(path) && fs::is_directory(path)) {
            watchPaths_.push_back(fs::canonical(path));
            std::cout << "Watching directory: " << watchPaths_.back() << std::endl;
        } else {
            std::cerr << "Path does not exist or is not a directory: " << path << std::endl;
        }
    }

    void setCallback(Callback cb) {
        callback_ = std::move(cb);
    }

    void start() {
        if (watchPaths_.empty()) {
            std::cerr << "No directories to watch." << std::endl;
            return;
        }

        if (!callback_) {
            std::cerr << "No callback set." << std::endl;
            return;
        }

        std::cout << "Starting file system watcher..." << std::endl;
        running_ = true;

        for (const auto& path : watchPaths_) {
            initializeFileStates(path);
        }

        while (running_) {
            checkForChanges();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void stop() {
        running_ = false;
        std::cout << "Stopping file system watcher..." << std::endl;
    }

private:
    struct FileState {
        std::time_t lastWriteTime;
        std::uintmax_t fileSize;
    };

    void initializeFileStates(const fs::path& directory) {
        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                auto lastWriteTime = fs::last_write_time(entry);
                auto fileSize = entry.file_size();
                fileStates_[entry.path()] = FileState{
                    std::chrono::system_clock::to_time_t(lastWriteTime),
                    fileSize
                };
            }
        }
    }

    void checkForChanges() {
        for (const auto& directory : watchPaths_) {
            for (const auto& entry : fs::recursive_directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    auto currentPath = entry.path();
                    auto currentLastWriteTime = fs::last_write_time(entry);
                    auto currentFileSize = entry.file_size();

                    auto it = fileStates_.find(currentPath);
                    if (it == fileStates_.end()) {
                        fileStates_[currentPath] = FileState{
                            std::chrono::system_clock::to_time_t(currentLastWriteTime),
                            currentFileSize
                        };
                        callback_(currentPath, "CREATED");
                    } else {
                        auto storedTime = it->second.lastWriteTime;
                        auto currentTime = std::chrono::system_clock::to_time_t(currentLastWriteTime);
                        if (currentTime != storedTime || currentFileSize != it->second.fileSize) {
                            it->second.lastWriteTime = currentTime;
                            it->second.fileSize = currentFileSize;
                            callback_(currentPath, "MODIFIED");
                        }
                    }
                }
            }

            std::vector<fs::path> pathsToRemove;
            for (const auto& [path, state] : fileStates_) {
                if (!fs::exists(path)) {
                    pathsToRemove.push_back(path);
                    callback_(path, "DELETED");
                }
            }

            for (const auto& path : pathsToRemove) {
                fileStates_.erase(path);
            }
        }
    }

    std::vector<fs::path> watchPaths_;
    std::unordered_map<fs::path, FileState> fileStates_;
    Callback callback_;
    bool running_{false};
};

int main() {
    FileSystemWatcher watcher;

    watcher.setCallback([](const fs::path& path, const std::string& action) {
        std::cout << "[" << action << "] " << path << std::endl;
    });

    watcher.addWatchPath(".");
    watcher.addWatchPath(fs::temp_directory_path());

    std::thread watcherThread([&watcher]() {
        watcher.start();
    });

    std::this_thread::sleep_for(std::chrono::seconds(30));
    watcher.stop();
    watcherThread.join();

    return 0;
}