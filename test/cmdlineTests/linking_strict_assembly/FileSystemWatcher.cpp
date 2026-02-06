#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& path) : watch_path(path) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        refresh_file_list();
    }

    void start_watching(int interval_seconds = 1) {
        std::cout << "Watching directory: " << watch_path << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path watch_path;
    std::unordered_set<std::string> known_files;

    void refresh_file_list() {
        known_files.clear();
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            known_files.insert(entry.path().filename().string());
        }
    }

    void check_for_changes() {
        std::unordered_set<std::string> current_files;
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            current_files.insert(entry.path().filename().string());
        }

        for (const auto& file : current_files) {
            if (known_files.find(file) == known_files.end()) {
                std::cout << "[ADDED] " << file << std::endl;
            }
        }

        for (const auto& file : known_files) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "[REMOVED] " << file << std::endl;
            }
        }

        known_files = std::move(current_files);
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
}#include <iostream>
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

    std::unordered_set<std::string> get_file_list() {
        std::unordered_set<std::string> files;
        for (const auto& entry : fs::directory_iterator(path_to_watch)) {
            files.insert(entry.path().filename().string());
        }
        return files;
    }

    void compare_and_log(const std::unordered_set<std::string>& old_files, const std::unordered_set<std::string>& new_files) {
        for (const auto& file : new_files) {
            if (old_files.find(file) == old_files.end()) {
                std::cout << "[+] File added: " << file << std::endl;
            }
        }
        for (const auto& file : old_files) {
            if (new_files.find(file) == new_files.end()) {
                std::cout << "[-] File removed: " << file << std::endl;
            }
        }
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        current_files = get_file_list();
        std::cout << "Watching directory: " << fs::absolute(path_to_watch) << std::endl;
        std::cout << "Initial file count: " << current_files.size() << std::endl;
    }

    void start(int interval_seconds = 2) {
        running = true;
        std::cout << "Starting watcher. Press Ctrl+C to stop." << std::endl;
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            auto new_files = get_file_list();
            compare_and_log(current_files, new_files);
            current_files = new_files;
        }
    }

    void stop() {
        running = false;
        std::cout << "Watcher stopped." << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}