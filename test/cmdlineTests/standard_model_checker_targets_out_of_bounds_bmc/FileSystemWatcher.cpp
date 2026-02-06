
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
}#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path path_to_watch;
    std::unordered_set<std::string> current_files;

    std::unordered_set<std::string> get_directory_contents() {
        std::unordered_set<std::string> files;
        for (const auto& entry : fs::directory_iterator(path_to_watch)) {
            files.insert(entry.path().filename().string());
        }
        return files;
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
            throw std::invalid_argument("Path does not exist or is not a directory");
        }
        current_files = get_directory_contents();
        std::cout << "Watching directory: " << fs::absolute(path_to_watch) << std::endl;
    }

    void start_monitoring(int interval_seconds = 2) {
        std::cout << "Monitoring started. Press Ctrl+C to stop." << std::endl;
        try {
            while (true) {
                std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
                auto new_files = get_directory_contents();

                // Check for added files
                for (const auto& file : new_files) {
                    if (current_files.find(file) == current_files.end()) {
                        std::cout << "[ADDED] " << file << std::endl;
                    }
                }

                // Check for removed files
                for (const auto& file : current_files) {
                    if (new_files.find(file) == new_files.end()) {
                        std::cout << "[REMOVED] " << file << std::endl;
                    }
                }

                current_files = std::move(new_files);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
};

int main(int argc, char* argv[]) {
    std::string path = ".";
    if (argc > 1) {
        path = argv[1];
    }

    try {
        FileSystemWatcher watcher(path);
        watcher.start_monitoring();
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize watcher: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}