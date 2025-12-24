
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    SimpleFileWatcher(const fs::path& path) : watch_path(path) {
        if (!fs::exists(path)) {
            throw std::runtime_error("Path does not exist: " + path.string());
        }
        last_check_time = fs::last_write_time(path);
        snapshot = collectFileSnapshot();
    }

    void startWatching(int interval_seconds = 1) {
        std::cout << "Watching: " << watch_path << std::endl;
        while (!should_stop) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            checkForChanges();
        }
    }

    void stopWatching() {
        should_stop = true;
    }

private:
    using FileSnapshot = std::unordered_map<std::string, fs::file_time_type>;

    FileSnapshot collectFileSnapshot() {
        FileSnapshot snap;
        if (fs::is_directory(watch_path)) {
            for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
                if (fs::is_regular_file(entry.path())) {
                    snap[entry.path().string()] = fs::last_write_time(entry.path());
                }
            }
        } else {
            snap[watch_path.string()] = fs::last_write_time(watch_path);
        }
        return snap;
    }

    void checkForChanges() {
        auto current_snapshot = collectFileSnapshot();
        auto current_time = fs::last_write_time(watch_path);

        if (current_time != last_check_time) {
            std::cout << "[Change] Root path modified: " << watch_path << std::endl;
            last_check_time = current_time;
        }

        for (const auto& [path, mtime] : current_snapshot) {
            auto it = snapshot.find(path);
            if (it == snapshot.end()) {
                std::cout << "[New] " << path << std::endl;
            } else if (it->second != mtime) {
                std::cout << "[Modified] " << path << std::endl;
            }
        }

        for (const auto& [path, mtime] : snapshot) {
            if (current_snapshot.find(path) == current_snapshot.end()) {
                std::cout << "[Deleted] " << path << std::endl;
            }
        }

        snapshot = std::move(current_snapshot);
    }

    fs::path watch_path;
    fs::file_time_type last_check_time;
    FileSnapshot snapshot;
    bool should_stop = false;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_watch>" << std::endl;
        return 1;
    }

    try {
        SimpleFileWatcher watcher(argv[1]);
        watcher.startWatching();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}