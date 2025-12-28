#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& path_to_watch) : path_to_watch_(path_to_watch) {
        if (!fs::exists(path_to_watch_) || !fs::is_directory(path_to_watch_)) {
            throw std::runtime_error("Path does not exist or is not a directory.");
        }
        refresh_file_list();
    }

    void start_watching(int interval_seconds = 1) {
        std::cout << "Watching directory: " << path_to_watch_ << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path path_to_watch_;
    std::unordered_set<std::string> current_files_;

    void refresh_file_list() {
        current_files_.clear();
        for (const auto& entry : fs::directory_iterator(path_to_watch_)) {
            current_files_.insert(entry.path().filename().string());
        }
    }

    void check_for_changes() {
        auto previous_files = current_files_;
        refresh_file_list();

        // Check for new files
        for (const auto& file : current_files_) {
            if (previous_files.find(file) == previous_files.end()) {
                std::cout << "[+] New file detected: " << file << std::endl;
            }
        }

        // Check for deleted files
        for (const auto& file : previous_files) {
            if (current_files_.find(file) == current_files_.end()) {
                std::cout << "[-] File deleted: " << file << std::endl;
            }
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start_watching(2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}