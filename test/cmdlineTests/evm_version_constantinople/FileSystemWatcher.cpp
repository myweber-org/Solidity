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

    std::unordered_set<std::string> get_files_in_directory() {
        std::unordered_set<std::string> files;
        if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                if (fs::is_regular_file(entry.status())) {
                    files.insert(entry.path().filename().string());
                }
            }
        }
        return files;
    }

    void compare_and_notify(const std::unordered_set<std::string>& new_files) {
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
        current_files = new_files;
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            current_files = get_files_in_directory();
            std::cout << "Watching directory: " << fs::absolute(path_to_watch) << std::endl;
            std::cout << "Initial file count: " << current_files.size() << std::endl;
        } else {
            std::cerr << "Error: Path does not exist or is not a directory." << std::endl;
        }
    }

    void start_watching(int interval_seconds = 2) {
        if (current_files.empty() && !(fs::exists(path_to_watch) && fs::is_directory(path_to_watch))) {
            return;
        }
        std::cout << "Starting watch loop (interval: " << interval_seconds << "s). Press Ctrl+C to stop." << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            auto new_files = get_files_in_directory();
            compare_and_notify(new_files);
        }
    }
};

int main(int argc, char* argv[]) {
    std::string path = ".";
    if (argc > 1) {
        path = argv[1];
    }
    FileSystemWatcher watcher(path);
    watcher.start_watching();
    return 0;
}