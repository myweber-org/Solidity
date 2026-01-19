#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& path) : watch_path_(path) {
        if (!fs::exists(watch_path_) || !fs::is_directory(watch_path_)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        populate_snapshot();
    }

    void start_watching(int interval_seconds = 1) {
        std::cout << "Watching directory: " << watch_path_ << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path watch_path_;
    std::unordered_map<std::string, fs::file_time_type> file_snapshot_;

    void populate_snapshot() {
        file_snapshot_.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (fs::is_regular_file(entry.status())) {
                file_snapshot_[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void check_for_changes() {
        auto current_state = file_snapshot_;
        bool changes_detected = false;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (!fs::is_regular_file(entry.status())) continue;

            std::string path_str = entry.path().string();
            auto current_time = fs::last_write_time(entry);

            if (file_snapshot_.find(path_str) == file_snapshot_.end()) {
                std::cout << "[NEW] File created: " << path_str << std::endl;
                changes_detected = true;
            } else if (file_snapshot_[path_str] != current_time) {
                std::cout << "[MODIFIED] File changed: " << path_str << std::endl;
                changes_detected = true;
            }
            current_state[path_str] = current_time;
        }

        for (const auto& [path, _] : file_snapshot_) {
            if (current_state.find(path) == current_state.end()) {
                std::cout << "[DELETED] File removed: " << path << std::endl;
                changes_detected = true;
            }
        }

        if (changes_detected) {
            file_snapshot_.swap(current_state);
        }
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
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const fs::path& path_to_watch) : path_to_watch_(fs::absolute(path_to_watch)) {
        if (!fs::exists(path_to_watch_) || !fs::is_directory(path_to_watch_)) {
            throw std::runtime_error("Provided path is not a valid directory.");
        }
        populate_file_map();
    }

    void start_monitoring(int interval_seconds = 1) {
        std::cout << "Starting to monitor: " << path_to_watch_ << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path path_to_watch_;
    std::unordered_set<std::string> current_files_;

    void populate_file_map() {
        current_files_.clear();
        for (const auto& entry : fs::directory_iterator(path_to_watch_)) {
            current_files_.insert(entry.path().filename().string());
        }
    }

    void check_for_changes() {
        auto previous_files = current_files_;
        populate_file_map();

        for (const auto& file : current_files_) {
            if (previous_files.find(file) == previous_files.end()) {
                std::cout << "[ADDED] " << file << std::endl;
            }
        }

        for (const auto& file : previous_files) {
            if (current_files_.find(file) == current_files_.end()) {
                std::cout << "[REMOVED] " << file << std::endl;
            }
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
        watcher.start_monitoring();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}