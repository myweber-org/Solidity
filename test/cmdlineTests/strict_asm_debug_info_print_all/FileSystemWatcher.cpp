#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <functional>
#include <atomic>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher() : running_(false) {}

    void addWatchPath(const fs::path& path) {
        if (fs::exists(path) && fs::is_directory(path)) {
            watch_paths_.push_back(path);
            updateSnapshot(path);
        }
    }

    void setCallback(Callback cb) {
        callback_ = std::move(cb);
    }

    void start() {
        running_ = true;
        worker_thread_ = std::thread(&FileSystemWatcher::watchLoop, this);
    }

    void stop() {
        running_ = false;
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

private:
    struct FileInfo {
        std::uintmax_t size;
        std::time_t last_write_time;
    };

    void updateSnapshot(const fs::path& path) {
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (fs::is_regular_file(entry.status())) {
                auto last_write = fs::last_write_time(entry.path());
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    last_write - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
                );
                std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

                file_snapshot_[entry.path()] = FileInfo{
                    fs::file_size(entry.path()),
                    cftime
                };
            }
        }
    }

    void watchLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            for (const auto& path : watch_paths_) {
                checkForChanges(path);
            }
        }
    }

    void checkForChanges(const fs::path& path) {
        std::unordered_map<fs::path, FileInfo> current_snapshot;

        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (fs::is_regular_file(entry.status())) {
                auto last_write = fs::last_write_time(entry.path());
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    last_write - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
                );
                std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

                current_snapshot[entry.path()] = FileInfo{
                    fs::file_size(entry.path()),
                    cftime
                };

                auto it = file_snapshot_.find(entry.path());
                if (it == file_snapshot_.end()) {
                    if (callback_) {
                        callback_(entry.path(), "CREATED");
                    }
                } else {
                    if (it->second.size != current_snapshot[entry.path()].size ||
                        it->second.last_write_time != current_snapshot[entry.path()].last_write_time) {
                        if (callback_) {
                            callback_(entry.path(), "MODIFIED");
                        }
                    }
                }
            }
        }

        for (const auto& [old_path, _] : file_snapshot_) {
            if (current_snapshot.find(old_path) == current_snapshot.end()) {
                if (callback_) {
                    callback_(old_path, "DELETED");
                }
            }
        }

        file_snapshot_.swap(current_snapshot);
    }

    std::vector<fs::path> watch_paths_;
    std::unordered_map<fs::path, FileInfo> file_snapshot_;
    Callback callback_;
    std::thread worker_thread_;
    std::atomic<bool> running_;
};

int main() {
    FileSystemWatcher watcher;

    watcher.setCallback([](const fs::path& path, const std::string& action) {
        std::cout << "File: " << path << " Action: " << action << std::endl;
    });

    watcher.addWatchPath(".");
    watcher.start();

    std::cout << "Watching for file changes. Press Enter to stop..." << std::endl;
    std::cin.get();

    watcher.stop();
    return 0;
}