
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

public:
    explicit FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
            throw std::runtime_error("Path does not exist or is not a directory");
        }
        refresh_file_list();
    }

    void refresh_file_list() {
        current_files.clear();
        for (const auto& entry : fs::directory_iterator(path_to_watch)) {
            current_files.insert(entry.path().filename().string());
        }
    }

    void watch(int interval_seconds = 1) {
        auto last_check = std::chrono::steady_clock::now();
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            auto current_files_snapshot = current_files;
            refresh_file_list();

            // Check for new files
            for (const auto& filename : current_files) {
                if (current_files_snapshot.find(filename) == current_files_snapshot.end()) {
                    std::cout << "New file detected: " << filename << std::endl;
                }
            }

            // Check for deleted files
            for (const auto& filename : current_files_snapshot) {
                if (current_files.find(filename) == current_files.end()) {
                    std::cout << "File deleted: " << filename << std::endl;
                }
            }

            last_check = std::chrono::steady_clock::now();
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        std::cout << "Watching current directory for file changes..." << std::endl;
        watcher.watch(2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}