#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileWatcher {
public:
    FileWatcher(const fs::path& path) : watch_path(path) {
        if (!fs::exists(path) || !fs::is_directory(path)) {
            throw std::runtime_error("Path does not exist or is not a directory");
        }
        updateSnapshot();
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

    void updateSnapshot() {
        file_snapshot.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_snapshot[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void checkForChanges() {
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (!fs::is_regular_file(entry.path())) continue;

            std::string file_path = entry.path().string();
            auto current_time = fs::last_write_time(entry.path());

            if (file_snapshot.find(file_path) == file_snapshot.end()) {
                std::cout << "New file detected: " << file_path << std::endl;
                file_snapshot[file_path] = current_time;
            } else if (file_snapshot[file_path] != current_time) {
                std::cout << "File modified: " << file_path << std::endl;
                file_snapshot[file_path] = current_time;
            }
        }

        std::vector<std::string> to_remove;
        for (const auto& [file_path, _] : file_snapshot) {
            if (!fs::exists(file_path)) {
                std::cout << "File deleted: " << file_path << std::endl;
                to_remove.push_back(file_path);
            }
        }

        for (const auto& file_path : to_remove) {
            file_snapshot.erase(file_path);
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        FileWatcher watcher(argv[1]);
        watcher.startWatching();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}