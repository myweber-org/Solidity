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
        if (fs::exists(path) && fs::is_directory(path)) {
            build_snapshot();
        } else {
            std::cerr << "Path does not exist or is not a directory: " << path << std::endl;
        }
    }

    void start_watching(int interval_seconds = 2) {
        std::cout << "Watching directory: " << watch_path << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_snapshot;

    void build_snapshot() {
        file_snapshot.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_snapshot[entry.path().string()] = fs::last_write_time(entry);
            }
        }
        std::cout << "Initial snapshot built with " << file_snapshot.size() << " files." << std::endl;
    }

    void check_for_changes() {
        std::unordered_map<std::string, fs::file_time_type> current_state;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                std::string file_path = entry.path().string();
                auto current_time = fs::last_write_time(entry);
                current_state[file_path] = current_time;

                auto it = file_snapshot.find(file_path);
                if (it == file_snapshot.end()) {
                    std::cout << "New file detected: " << file_path << std::endl;
                } else if (it->second != current_time) {
                    std::cout << "File modified: " << file_path << std::endl;
                }
            }
        }

        for (const auto& [old_file, _] : file_snapshot) {
            if (current_state.find(old_file) == current_state.end()) {
                std::cout << "File deleted: " << old_file << std::endl;
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