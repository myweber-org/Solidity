#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& directory) : watch_path(directory) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        update_snapshot();
    }

    void start_monitoring(int interval_seconds = 2) {
        std::cout << "Starting to monitor: " << watch_path.string() << std::endl;
        std::cout << "Checking every " << interval_seconds << " seconds." << std::endl;

        while (monitoring_active) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

    void stop_monitoring() {
        monitoring_active = false;
    }

private:
    fs::path watch_path;
    std::unordered_set<std::string> file_snapshot;
    bool monitoring_active{true};

    void update_snapshot() {
        file_snapshot.clear();
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.status())) {
                file_snapshot.insert(entry.path().filename().string());
            }
        }
    }

    void check_for_changes() {
        std::unordered_set<std::string> current_files;

        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.status())) {
                current_files.insert(entry.path().filename().string());
            }
        }

        for (const auto& file : current_files) {
            if (file_snapshot.find(file) == file_snapshot.end()) {
                std::cout << "[+] New file detected: " << file << std::endl;
            }
        }

        for (const auto& file : file_snapshot) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "[-] File removed: " << file << std::endl;
            }
        }

        file_snapshot = std::move(current_files);
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start_monitoring(3);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}