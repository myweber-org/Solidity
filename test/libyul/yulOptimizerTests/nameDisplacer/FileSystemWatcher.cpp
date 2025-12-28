
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
    bool running = true;

    std::unordered_set<std::string> get_directory_contents() {
        std::unordered_set<std::string> files;
        if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                files.insert(entry.path().filename().string());
            }
        }
        return files;
    }

    void compare_and_log(const std::unordered_set<std::string>& old_files,
                         const std::unordered_set<std::string>& new_files) {
        for (const auto& file : new_files) {
            if (old_files.find(file) == old_files.end()) {
                std::cout << "[ADDED] " << file << std::endl;
            }
        }
        for (const auto& file : old_files) {
            if (new_files.find(file) == new_files.end()) {
                std::cout << "[REMOVED] " << file << std::endl;
            }
        }
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!fs::exists(path_to_watch)) {
            throw std::runtime_error("Path does not exist: " + path);
        }
        current_files = get_directory_contents();
        std::cout << "Watching directory: " << fs::absolute(path_to_watch) << std::endl;
        std::cout << "Initial file count: " << current_files.size() << std::endl;
    }

    void start_watching(int interval_seconds = 2) {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            auto new_files = get_directory_contents();
            compare_and_log(current_files, new_files);
            current_files = new_files;
        }
    }

    void stop() {
        running = false;
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start_watching(1);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}