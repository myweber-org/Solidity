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
        for (const auto& entry : fs::directory_iterator(path_to_watch)) {
            current_files.insert(entry.path().filename().string());
        }
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        populate_file_set();
    }

    void start(int interval_seconds = 1) {
        running = true;
        std::cout << "Starting to watch: " << path_to_watch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            auto old_files = current_files;
            populate_file_set();

            for (const auto& fname : current_files) {
                if (old_files.find(fname) == old_files.end()) {
                    std::cout << "File added: " << fname << std::endl;
                }
            }

            for (const auto& fname : old_files) {
                if (current_files.find(fname) == current_files.end()) {
                    std::cout << "File removed: " << fname << std::endl;
                }
            }
        }
    }

    void stop() {
        running = false;
        std::cout << "Stopped watching." << std::endl;
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start(2);
        std::this_thread::sleep_for(std::chrono::seconds(10));
        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}