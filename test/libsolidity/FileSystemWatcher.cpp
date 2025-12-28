
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const std::string& path) : watch_path(path) {
        if (fs::exists(path) && fs::is_directory(path)) {
            populate_file_map();
        }
    }

    void start_watching(int interval_seconds = 2) {
        std::cout << "Watching directory: " << watch_path << std::endl;
        std::cout << "Polling interval: " << interval_seconds << " seconds" << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    std::string watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_map;

    void populate_file_map() {
        file_map.clear();
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_map[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void check_for_changes() {
        std::unordered_map<std::string, fs::file_time_type> current_map;

        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                std::string file_path = entry.path().string();
                auto current_time = fs::last_write_time(entry.path());
                current_map[file_path] = current_time;

                if (file_map.find(file_path) == file_map.end()) {
                    std::cout << "[NEW] " << file_path << std::endl;
                } else if (file_map[file_path] != current_time) {
                    std::cout << "[MODIFIED] " << file_path << std::endl;
                }
            }
        }

        for (const auto& [file_path, _] : file_map) {
            if (current_map.find(file_path) == current_map.end()) {
                std::cout << "[DELETED] " << file_path << std::endl;
            }
        }

        file_map = std::move(current_map);
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
    explicit FileSystemWatcher(const fs::path& path_to_watch) : path_to_watch_(path_to_watch) {
        if (!fs::exists(path_to_watch_) || !fs::is_directory(path_to_watch_)) {
            throw std::runtime_error("Provided path is not a valid directory.");
        }
        refresh_file_list();
    }

    void start_monitoring(int interval_seconds = 1) {
        std::cout << "Starting to monitor: " << path_to_watch_.string() << std::endl;
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

        for (const auto& file : current_files_) {
            if (previous_files.find(file) == previous_files.end()) {
                std::cout << "[+] File added: " << file << std::endl;
            }
        }

        for (const auto& file : previous_files) {
            if (current_files_.find(file) == current_files_.end()) {
                std::cout << "[-] File removed: " << file << std::endl;
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