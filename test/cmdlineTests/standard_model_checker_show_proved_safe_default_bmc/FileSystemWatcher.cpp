
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    using FileTimeMap = std::unordered_map<std::string, fs::file_time_type>;

    SimpleFileWatcher(const std::string& path) : watch_path(path), running(false) {}

    void start() {
        if (!fs::exists(watch_path)) {
            std::cerr << "Path does not exist: " << watch_path << std::endl;
            return;
        }

        running = true;
        snapshot_files();
        std::cout << "Watching directory: " << watch_path << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            check_for_changes();
        }
    }

    void stop() {
        running = false;
    }

private:
    std::string watch_path;
    bool running;
    FileTimeMap file_timestamps;

    void snapshot_files() {
        file_timestamps.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                std::string file_path = entry.path().string();
                file_timestamps[file_path] = fs::last_write_time(entry);
            }
        }
    }

    void check_for_changes() {
        FileTimeMap current_state;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                std::string file_path = entry.path().string();
                current_state[file_path] = fs::last_write_time(entry);

                auto old_it = file_timestamps.find(file_path);
                if (old_it == file_timestamps.end()) {
                    std::cout << "File created: " << file_path << std::endl;
                } else if (old_it->second != current_state[file_path]) {
                    std::cout << "File modified: " << file_path << std::endl;
                }
            }
        }

        for (const auto& old_file : file_timestamps) {
            if (current_state.find(old_file.first) == current_state.end()) {
                std::cout << "File deleted: " << old_file.first << std::endl;
            }
        }

        file_timestamps.swap(current_state);
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    SimpleFileWatcher watcher(argv[1]);
    watcher.start();

    return 0;
}