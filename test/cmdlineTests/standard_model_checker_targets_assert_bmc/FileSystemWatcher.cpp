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
    bool running = false;

    void populate_file_set() {
        current_files.clear();
        if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                current_files.insert(entry.path().filename().string());
            }
        }
    }

public:
    explicit FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        populate_file_set();
    }

    void start_watching(int interval_seconds = 2) {
        running = true;
        std::cout << "Starting to watch: " << path_to_wwatch.string() << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
                std::cerr << "Path is not accessible or not a directory." << std::endl;
                break;
            }

            std::unordered_set<std::string> new_files;
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                new_files.insert(entry.path().filename().string());
            }

            for (const auto& file : new_files) {
                if (current_files.find(file) == current_files.end()) {
                    std::cout << "File added: " << file << std::endl;
                }
            }

            for (const auto& file : current_files) {
                if (new_files.find(file) == new_files.end()) {
                    std::cout << "File removed: " << file << std::endl;
                }
            }

            current_files = std::move(new_files);
        }
    }

    void stop_watching() {
        running = false;
    }
};

int main(int argc, char* argv[]) {
    std::string path = ".";
    if (argc > 1) {
        path = argv[1];
    }

    FileSystemWatcher watcher(path);
    watcher.start_watching();

    return 0;
}
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>
#include <iostream>
#include <functional>

namespace fs = std::filesystem;

class FileWatcher {
public:
    using FileChangeCallback = std::function<void(const fs::path&, const std::string&)>;

    FileWatcher(const fs::path& directory, std::chrono::milliseconds interval)
        : watch_directory(directory), poll_interval(interval), running(false) {}

    void start(const FileChangeCallback& callback) {
        if (!fs::exists(watch_directory) || !fs::is_directory(watch_directory)) {
            std::cerr << "Directory does not exist or is not accessible: " << watch_directory << std::endl;
            return;
        }

        running = true;
        snapshot_files();

        while (running) {
            std::this_thread::sleep_for(poll_interval);
            check_for_changes(callback);
        }
    }

    void stop() {
        running = false;
    }

private:
    fs::path watch_directory;
    std::chrono::milliseconds poll_interval;
    bool running;
    std::unordered_map<std::string, fs::file_time_type> file_snapshot;

    void snapshot_files() {
        file_snapshot.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.status())) {
                file_snapshot[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void check_for_changes(const FileChangeCallback& callback) {
        std::unordered_map<std::string, fs::file_time_type> current_state;

        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.status())) {
                std::string path_str = entry.path().string();
                current_state[path_str] = fs::last_write_time(entry);

                auto it = file_snapshot.find(path_str);
                if (it == file_snapshot.end()) {
                    callback(entry.path(), "created");
                } else if (it->second != current_state[path_str]) {
                    callback(entry.path(), "modified");
                }
            }
        }

        for (const auto& [path, _] : file_snapshot) {
            if (current_state.find(path) == current_state.end()) {
                callback(fs::path(path), "deleted");
            }
        }

        file_snapshot = std::move(current_state);
    }
};

int main() {
    FileWatcher watcher(fs::current_path(), std::chrono::seconds(2));

    watcher.start([](const fs::path& path, const std::string& change_type) {
        std::cout << "File " << path.filename() << " has been " << change_type << std::endl;
    });

    return 0;
}