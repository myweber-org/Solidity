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
        std::cout << "Watching directory: " << path_to_watch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            if (!fs::exists(path_to_watch)) {
                std::cout << "Directory deleted or inaccessible." << std::endl;
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

int main() {
    std::string watch_path = ".";
    FileSystemWatcher watcher(watch_path);

    std::thread watch_thread([&watcher]() {
        watcher.start_watching();
    });

    std::cout << "File system watcher started. Press Enter to stop..." << std::endl;
    std::cin.get();

    watcher.stop_watching();
    if (watch_thread.joinable()) {
        watch_thread.join();
    }

    std::cout << "Watcher stopped." << std::endl;
    return 0;
}