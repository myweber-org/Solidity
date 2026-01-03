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

    void populateFileSet() {
        current_files.clear();
        if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                current_files.insert(entry.path().filename().string());
            }
        }
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        populateFileSet();
    }

    void startMonitoring(int interval_seconds = 2) {
        std::cout << "Starting to monitor: " << path_to_watch << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            checkForChanges();
        }
    }

    void checkForChanges() {
        auto old_files = current_files;
        populateFileSet();

        // Check for new files
        for (const auto& file : current_files) {
            if (old_files.find(file) == old_files.end()) {
                std::cout << "[+] New file detected: " << file << std::endl;
            }
        }

        // Check for deleted files
        for (const auto& file : old_files) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "[-] File deleted: " << file << std::endl;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    std::string watch_path = argv[1];
    FileSystemWatcher watcher(watch_path);
    watcher.startMonitoring();

    return 0;
}