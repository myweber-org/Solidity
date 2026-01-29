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
                if (fs::is_regular_file(entry.status())) {
                    current_files.insert(entry.path().filename().string());
                }
            }
        }
    }

public:
    explicit FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        populateFileSet();
    }

    void startMonitoring(int interval_seconds = 2) {
        std::cout << "Starting to monitor: " << path_to_watch << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            auto old_files = current_files;
            populateFileSet();

            for (const auto& file : current_files) {
                if (old_files.find(file) == old_files.end()) {
                    std::cout << "[+] File added: " << file << std::endl;
                }
            }

            for (const auto& file : old_files) {
                if (current_files.find(file) == current_files.end()) {
                    std::cout << "[-] File removed: " << file << std::endl;
                }
            }
        }
    }
};

int main() {
    std::string watch_path = ".";
    FileSystemWatcher watcher(watch_path);
    watcher.startMonitoring();
    return 0;
}