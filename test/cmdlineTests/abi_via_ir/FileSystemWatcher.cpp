#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path path_to_watch;
    std::unordered_set<std::string> current_files;

    std::unordered_set<std::string> get_files_in_directory() {
        std::unordered_set<std::string> files;
        for (const auto& entry : fs::directory_iterator(path_to_watch)) {
            if (entry.is_regular_file()) {
                files.insert(entry.path().filename().string());
            }
        }
        return files;
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        current_files = get_files_in_directory();
        std::cout << "Watching directory: " << fs::absolute(path_to_watch) << std::endl;
    }

    void start_monitoring(int interval_seconds = 2) {
        std::cout << "Monitoring started. Press Ctrl+C to stop." << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            auto new_files = get_files_in_directory();

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

            current_files = new_files;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start_monitoring();
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
            watchPaths.insert(path);
        }
    }

    void setCallback(Callback cb) {
        callback = std::move(cb);
    }

    void startMonitoring(int intervalSeconds = 1) {
        monitoring = true;
        while (monitoring) {
            checkForChanges();
            std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
        }
    }

    void stopMonitoring() {
        monitoring = false;
    }

private:
    std::unordered_map<FilePath, FileTime> fileStates;
    std::unordered_set<FilePath> watchPaths;
    Callback callback;
    bool monitoring = false;

    void updateFileState(const FilePath& path) {
        if (fs::exists(path)) {
            fileStates[path] = fs::last_write_time(path);
        }
    }

    void checkForChanges() {
        for (const auto& path : watchPaths) {
            if (!fs::exists(path)) {
                if (fileStates.find(path) != fileStates.end()) {
                    fileStates.erase(path);
                    if (callback) {
                        callback(path, "deleted");
                    }
                }
                continue;
            }

            auto currentTime = fs::last_write_time(path);
            auto it = fileStates.find(path);

            if (it == fileStates.end()) {
                fileStates[path] = currentTime;
                if (callback) {
                    callback(path, "created");
                }
            } else if (it->second != currentTime) {
                it->second = currentTime;
                if (callback) {
                    callback(path, "modified");
                }
            }
        }
    }
};

void exampleUsage() {
    FileSystemWatcher watcher;

    watcher.setCallback([](const FileSystemWatcher::FilePath& path, const std::string& action) {
        std::cout << "File: " << path.string() << " Action: " << action << std::endl;
    });

    watcher.addWatchPath("test_directory");
    watcher.addWatchPath("example.txt");

    std::thread monitorThread([&watcher]() {
        watcher.startMonitoring(2);
    });

    std::this_thread::sleep_for(std::chrono::seconds(10));
    watcher.stopMonitoring();
    monitorThread.join();
}

int main() {
    exampleUsage();
    return 0;
}