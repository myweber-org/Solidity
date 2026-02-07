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
        if (!fs::exists(path_to_watch)) {
            fs::create_directories(path_to_watch);
        }
        populateFileSet();
        std::cout << "Watching directory: " << fs::absolute(path_to_watch) << std::endl;
    }

    void startMonitoring(int interval_seconds = 2) {
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

int main(int argc, char* argv[]) {
    std::string path = ".";
    if (argc > 1) {
        path = argv[1];
    }

    try {
        FileSystemWatcher watcher(path);
        watcher.startMonitoring();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}