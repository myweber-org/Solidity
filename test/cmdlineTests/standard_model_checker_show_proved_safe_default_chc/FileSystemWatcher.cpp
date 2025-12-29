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
        snapshot_files();
    }

    void start_watching(std::chrono::milliseconds interval = std::chrono::milliseconds(1000)) {
        std::cout << "Watching directory: " << watch_path.string() << std::endl;
        
        while (running) {
            std::this_thread::sleep_for(interval);
            check_for_changes();
        }
    }

    void stop_watching() {
        running = false;
    }

private:
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_snapshot;
    bool running = true;

    void snapshot_files() {
        file_snapshot.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_snapshot[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void check_for_changes() {
        auto current_files = std::unordered_map<std::string, fs::file_time_type>{};
        
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                current_files[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }

        for (const auto& [path, timestamp] : current_files) {
            if (file_snapshot.find(path) == file_snapshot.end()) {
                std::cout << "[CREATED] " << path << std::endl;
            } else if (file_snapshot[path] != timestamp) {
                std::cout << "[MODIFIED] " << path << std::endl;
            }
        }

        for (const auto& [path, timestamp] : file_snapshot) {
            if (current_files.find(path) == current_files.end()) {
                std::cout << "[DELETED] " << path << std::endl;
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
        watcher.start_watching();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}