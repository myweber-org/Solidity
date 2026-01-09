#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const fs::path& path_to_watch) : path_to_watch_(path_to_watch) {
        if (!fs::exists(path_to_watch_) || !fs::is_directory(path_to_watch_)) {
            throw std::runtime_error("Provided path is not a valid directory.");
        }
        populate_file_set();
    }

    void start_watching(int interval_seconds = 1) {
        std::cout << "Starting to watch: " << path_to_watch_ << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path path_to_watch_;
    std::unordered_set<std::string> current_files_;

    void populate_file_set() {
        current_files_.clear();
        for (const auto& entry : fs::directory_iterator(path_to_watch_)) {
            current_files_.insert(entry.path().filename().string());
        }
    }

    void check_for_changes() {
        auto new_files = std::unordered_set<std::string>();
        bool changed = false;

        for (const auto& entry : fs::directory_iterator(path_to_watch_)) {
            std::string filename = entry.path().filename().string();
            new_files.insert(filename);

            if (current_files_.find(filename) == current_files_.end()) {
                std::cout << "File added: " << filename << std::endl;
                changed = true;
            }
        }

        for (const auto& old_file : current_files_) {
            if (new_files.find(old_file) == new_files.end()) {
                std::cout << "File removed: " << old_file << std::endl;
                changed = true;
            }
        }

        if (changed) {
            current_files_.swap(new_files);
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
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