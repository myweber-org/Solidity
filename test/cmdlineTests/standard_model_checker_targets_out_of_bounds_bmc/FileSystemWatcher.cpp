
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
            watchPaths[path] = getLastWriteTime(path);
        }
    }

    void setCallback(Callback cb) {
        callback = std::move(cb);
    }

    void startWatching(int intervalMs = 1000) {
        running = true;
        while (running) {
            checkForChanges();
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
        }
    }

    void stopWatching() {
        running = false;
    }

private:
    std::unordered_map<fs::path, fs::file_time_type> watchPaths;
    Callback callback;
    bool running = false;

    fs::file_time_type getLastWriteTime(const fs::path& path) {
        if (fs::exists(path)) {
            return fs::last_write_time(path);
        }
        return fs::file_time_type::min();
    }

    void checkForChanges() {
        for (auto& [path, lastTime] : watchPaths) {
            if (!fs::exists(path)) continue;

            auto currentTime = getLastWriteTime(path);
            if (currentTime != lastTime) {
                lastTime = currentTime;
                if (callback) {
                    callback(path, "modified");
                }
            }

            if (fs::is_directory(path)) {
                for (const auto& entry : fs::recursive_directory_iterator(path)) {
                    auto entryPath = entry.path();
                    if (watchPaths.find(entryPath) == watchPaths.end()) {
                        watchPaths[entryPath] = getLastWriteTime(entryPath);
                        if (callback) {
                            callback(entryPath, "created");
                        }
                    }
                }
            }
        }

        auto it = watchPaths.begin();
        while (it != watchPaths.end()) {
            if (!fs::exists(it->first)) {
                if (callback) {
                    callback(it->first, "deleted");
                }
                it = watchPaths.erase(it);
            } else {
                ++it;
            }
        }
    }
};

int main() {
    FileSystemWatcher watcher;

    watcher.setCallback([](const fs::path& path, const std::string& action) {
        std::cout << "File " << path << " has been " << action << std::endl;
    });

    watcher.addWatchPath(".");
    
    std::thread watchThread([&watcher]() {
        watcher.startWatching(500);
    });

    std::cout << "Watching current directory. Press Enter to stop..." << std::endl;
    std::cin.get();

    watcher.stopWatching();
    watchThread.join();

    return 0;
}