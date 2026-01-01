
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <iostream>
#include <functional>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using FileTime = fs::file_time_type;
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher() = default;

    void addWatchPath(const fs::path& path) {
        if (!fs::exists(path)) {
            std::cerr << "Path does not exist: " << path << std::endl;
            return;
        }

        if (fs::is_directory(path)) {
            watchPaths_.push_back(fs::canonical(path));
        } else {
            std::cerr << "Path is not a directory: " << path << std::endl;
        }
    }

    void setCallback(Callback cb) {
        callback_ = std::move(cb);
    }

    void start() {
        running_ = true;
        initializeFileTimes();
        monitorThread_ = std::thread(&FileSystemWatcher::monitor, this);
    }

    void stop() {
        running_ = false;
        if (monitorThread_.joinable()) {
            monitorThread_.join();
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

private:
    void initializeFileTimes() {
        fileTimes_.clear();
        for (const auto& watchPath : watchPaths_) {
            for (const auto& entry : fs::recursive_directory_iterator(watchPath)) {
                if (fs::is_regular_file(entry.path())) {
                    try {
                        fileTimes_[entry.path()] = fs::last_write_time(entry.path());
                    } catch (const fs::filesystem_error&) {
                        // Skip files we can't access
                    }
                }
            }
        }
    }

    void monitor() {
        while (running_) {
            std::this_thread::sleep_for(pollInterval_);

            std::unordered_map<fs::path, FileTime> currentFileTimes;

            // Collect current file times
            for (const auto& watchPath : watchPaths_) {
                if (!fs::exists(watchPath)) continue;

                for (const auto& entry : fs::recursive_directory_iterator(watchPath)) {
                    if (fs::is_regular_file(entry.path())) {
                        try {
                            currentFileTimes[entry.path()] = fs::last_write_time(entry.path());
                        } catch (const fs::filesystem_error&) {
                            continue;
                        }
                    }
                }
            }

            // Check for new or modified files
            for (const auto& [path, currentTime] : currentFileTimes) {
                auto it = fileTimes_.find(path);
                if (it == fileTimes_.end()) {
                    // New file
                    if (callback_) {
                        callback_(path, "created");
                    }
                } else if (it->second != currentTime) {
                    // Modified file
                    if (callback_) {
                        callback_(path, "modified");
                    }
                }
            }

            // Check for deleted files
            for (const auto& [path, oldTime] : fileTimes_) {
                if (currentFileTimes.find(path) == currentFileTimes.end()) {
                    // Deleted file
                    if (callback_) {
                        callback_(path, "deleted");
                    }
                }
            }

            // Update stored file times
            fileTimes_ = std::move(currentFileTimes);
        }
    }

    std::vector<fs::path> watchPaths_;
    std::unordered_map<fs::path, FileTime> fileTimes_;
    Callback callback_;
    std::thread monitorThread_;
    std::chrono::milliseconds pollInterval_{500};
    bool running_{false};
};

// Example usage
int main() {
    FileSystemWatcher watcher;

    watcher.addWatchPath(".");
    watcher.addWatchPath("/tmp");

    watcher.setCallback([](const fs::path& path, const std::string& action) {
        std::cout << "File " << path << " was " << action << std::endl;
    });

    watcher.start();

    // Run for 30 seconds
    std::this_thread::sleep_for(std::chrono::seconds(30));

    watcher.stop();

    return 0;
}