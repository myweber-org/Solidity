#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& directory) : watch_directory(directory) {
        if (!fs::exists(watch_directory) || !fs::is_directory(watch_directory)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        update_file_set();
    }

    void start_watching(int interval_seconds = 2) {
        std::cout << "Watching directory: " << watch_directory << std::endl;
        std::cout << "Press Ctrl+C to stop." << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path watch_directory;
    std::unordered_set<std::string> known_files;

    void update_file_set() {
        known_files.clear();
        for (const auto& entry : fs::directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.path())) {
                known_files.insert(entry.path().filename().string());
            }
        }
    }

    void check_for_changes() {
        auto current_files = std::unordered_set<std::string>{};
        for (const auto& entry : fs::directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.path())) {
                current_files.insert(entry.path().filename().string());
            }
        }

        for (const auto& file : current_files) {
            if (known_files.find(file) == known_files.end()) {
                std::cout << "New file detected: " << file << std::endl;
            }
        }

        for (const auto& file : known_files) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "File removed: " << file << std::endl;
            }
        }

        known_files = std::move(current_files);
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
}#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const fs::path& path) : watch_path(path) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Path does not exist or is not a directory");
        }
        snapshot();
    }

    void start(int interval_seconds = 1) {
        std::cout << "Watching: " << watch_path.string() << std::endl;
        running = true;
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check();
        }
    }

    void stop() {
        running = false;
    }

private:
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps;
    bool running = false;

    void snapshot() {
        file_timestamps.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_timestamps[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void check() {
        std::unordered_map<std::string, fs::file_time_type> current;
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                current[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }

        for (const auto& [path, time] : current) {
            if (file_timestamps.find(path) == file_timestamps.end()) {
                std::cout << "Created: " << path << std::endl;
            } else if (file_timestamps[path] != time) {
                std::cout << "Modified: " << path << std::endl;
            }
        }

        for (const auto& [path, time] : file_timestamps) {
            if (current.find(path) == current.end()) {
                std::cout << "Deleted: " << path << std::endl;
            }
        }

        file_timestamps = std::move(current);
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        std::thread watch_thread([&watcher]() {
            watcher.start(2);
        });

        std::cout << "Press Enter to stop watching..." << std::endl;
        std::cin.get();
        watcher.stop();
        watch_thread.join();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}