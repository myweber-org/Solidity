#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const fs::path& path) : watch_path(path) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Invalid directory path");
        }
        populate_snapshot();
    }

    void start_monitoring(int interval_seconds = 2) {
        std::cout << "Monitoring directory: " << watch_path << std::endl;
        std::cout << "Press Ctrl+C to stop..." << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path watch_path;
    std::unordered_set<std::string> file_snapshot;

    void populate_snapshot() {
        file_snapshot.clear();
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            file_snapshot.insert(entry.path().filename().string());
        }
    }

    void check_for_changes() {
        auto current_files = get_current_files();
        
        std::unordered_set<std::string> added_files;
        std::unordered_set<std::string> removed_files;

        for (const auto& file : current_files) {
            if (file_snapshot.find(file) == file_snapshot.end()) {
                added_files.insert(file);
            }
        }

        for (const auto& file : file_snapshot) {
            if (current_files.find(file) == current_files.end()) {
                removed_files.insert(file);
            }
        }

        if (!added_files.empty() || !removed_files.empty()) {
            for (const auto& file : added_files) {
                std::cout << "[+] File added: " << file << std::endl;
            }
            for (const auto& file : removed_files) {
                std::cout << "[-] File removed: " << file << std::endl;
            }
            file_snapshot = std::move(current_files);
        }
    }

    std::unordered_set<std::string> get_current_files() {
        std::unordered_set<std::string> current;
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            current.insert(entry.path().filename().string());
        }
        return current;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
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