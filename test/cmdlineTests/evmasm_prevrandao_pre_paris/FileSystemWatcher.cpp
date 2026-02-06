
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& path) : watch_path(path) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Path does not exist or is not a directory");
        }
        populate_snapshot();
    }

    void start_watching(int interval_seconds = 1) {
        std::cout << "Watching directory: " << watch_path.string() << std::endl;
        while (!stop_requested) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

    void stop() {
        stop_requested = true;
    }

private:
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_snapshot;
    bool stop_requested = false;

    void populate_snapshot() {
        file_snapshot.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_snapshot[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void check_for_changes() {
        std::unordered_map<std::string, fs::file_time_type> current_state;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                std::string file_path = entry.path().string();
                auto current_time = fs::last_write_time(entry.path());
                current_state[file_path] = current_time;

                auto it = file_snapshot.find(file_path);
                if (it == file_snapshot.end()) {
                    std::cout << "[NEW] " << file_path << std::endl;
                } else if (it->second != current_time) {
                    std::cout << "[MODIFIED] " << file_path << std::endl;
                }
            }
        }

        for (const auto& [old_path, old_time] : file_snapshot) {
            if (current_state.find(old_path) == current_state.end()) {
                std::cout << "[DELETED] " << old_path << std::endl;
            }
        }

        file_snapshot = std::move(current_state);
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start_watching();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <mutex>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const fs::file_time_type&)>;

    FileSystemWatcher() : running_(false) {}

    void addWatchPath(const fs::path& path, Callback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (fs::exists(path)) {
            auto lastWriteTime = fs::last_write_time(path);
            watchMap_[path] = {callback, lastWriteTime};
            std::cout << "Watching: " << path.string() << std::endl;
        } else {
            std::cerr << "Path does not exist: " << path.string() << std::endl;
        }
    }

    void start() {
        running_ = true;
        monitorThread_ = std::thread(&FileSystemWatcher::monitorLoop, this);
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
    struct WatchInfo {
        Callback callback;
        fs::file_time_type lastWriteTime;
    };

    std::unordered_map<fs::path, WatchInfo> watchMap_;
    std::atomic<bool> running_;
    std::thread monitorThread_;
    std::mutex mutex_;

    void monitorLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            std::lock_guard<std::mutex> lock(mutex_);
            for (auto& [path, info] : watchMap_) {
                if (fs::exists(path)) {
                    auto currentWriteTime = fs::last_write_time(path);
                    if (currentWriteTime != info.lastWriteTime) {
                        info.callback(path, currentWriteTime);
                        info.lastWriteTime = currentWriteTime;
                    }
                }
            }
        }
    }
};

void exampleCallback(const fs::path& path, const fs::file_time_type& time) {
    auto timeT = std::chrono::system_clock::to_time_t(
        std::chrono::file_clock::to_sys(time)
    );
    std::cout << "File modified: " << path.filename().string()
              << " at " << std::ctime(&timeT);
}

int main() {
    FileSystemWatcher watcher;

    watcher.addWatchPath("config.json", exampleCallback);
    watcher.addWatchPath("logs/", [](const fs::path& p, const fs::file_time_type& t) {
        std::cout << "Directory content changed: " << p.string() << std::endl;
    });

    watcher.start();

    std::cout << "File system watcher started. Press Enter to stop..." << std::endl;
    std::cin.get();

    watcher.stop();
    return 0;
}