
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& path) : watch_path(path) {
        if (!fs::exists(path) || !fs::is_directory(path)) {
            throw std::runtime_error("Path does not exist or is not a directory");
        }
        updateSnapshot();
    }

    void startWatching(int interval_seconds = 2) {
        std::cout << "Watching directory: " << watch_path << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            checkForChanges();
        }
    }

private:
    fs::path watch_path;
    std::unordered_set<std::string> last_snapshot;

    void updateSnapshot() {
        last_snapshot.clear();
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            last_snapshot.insert(entry.path().filename().string());
        }
    }

    void checkForChanges() {
        auto current_snapshot = std::unordered_set<std::string>();
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            current_snapshot.insert(entry.path().filename().string());
        }

        for (const auto& file : current_snapshot) {
            if (last_snapshot.find(file) == last_snapshot.end()) {
                std::cout << "File added: " << file << std::endl;
            }
        }

        for (const auto& file : last_snapshot) {
            if (current_snapshot.find(file) == current_snapshot.end()) {
                std::cout << "File removed: " << file << std::endl;
            }
        }

        last_snapshot = std::move(current_snapshot);
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.startWatching(1);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}