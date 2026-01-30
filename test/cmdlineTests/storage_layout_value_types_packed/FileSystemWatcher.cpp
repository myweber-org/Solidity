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
                if (fs::is_regular_file(entry.status())) {
                    current_files.insert(entry.path().filename().string());
                }
            }
        }
    }

public:
    explicit FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        populate_file_set();
    }

    void start_watching(int interval_seconds = 2) {
        running = true;
        std::cout << "Starting to watch: " << path_to_watch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            if (!fs::exists(path_to_watch)) {
                std::cout << "Watched directory no longer exists. Stopping." << std::endl;
                stop();
                break;
            }

            std::unordered_set<std::string> new_files;
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                if (fs::is_regular_file(entry.status())) {
                    std::string filename = entry.path().filename().string();
                    new_files.insert(filename);

                    if (current_files.find(filename) == current_files.end()) {
                        std::cout << "File added: " << filename << std::endl;
                    }
                }
            }

            for (const auto& old_file : current_files) {
                if (new_files.find(old_file) == new_files.end()) {
                    std::cout << "File removed: " << old_file << std::endl;
                }
            }

            current_files = std::move(new_files);
        }
    }

    void stop() {
        running = false;
    }
};

int main() {
    std::string watch_path = ".";
    FileSystemWatcher watcher(watch_path);

    watcher.start_watching();
    return 0;
}