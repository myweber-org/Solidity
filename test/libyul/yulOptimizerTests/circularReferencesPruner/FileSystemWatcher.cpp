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
        populateSnapshot();
    }

    void startWatching(int interval_seconds = 1) {
        std::cout << "Watching directory: " << watch_path << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            checkForChanges();
        }
    }

private:
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_snapshot;

    void populateSnapshot() {
        file_snapshot.clear();
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_snapshot[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void checkForChanges() {
        auto current_files = std::unordered_map<std::string, fs::file_time_type>{};
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                current_files[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }

        for (const auto& [path, time] : current_files) {
            if (file_snapshot.find(path) == file_snapshot.end()) {
                std::cout << "File created: " << path << std::endl;
            } else if (file_snapshot[path] != time) {
                std::cout << "File modified: " << path << std::endl;
            }
        }

        for (const auto& [path, time] : file_snapshot) {
            if (current_files.find(path) == current_files.end()) {
                std::cout << "File deleted: " << path << std::endl;
            }
        }

        file_snapshot = std::move(current_files);
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.startWatching();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}