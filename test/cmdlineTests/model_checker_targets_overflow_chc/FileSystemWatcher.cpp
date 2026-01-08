#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>
#include <string>

namespace fs = std::filesystem;

class DirectoryWatcher {
private:
    fs::path watch_path;
    std::unordered_set<std::string> current_files;

    void scan_directory() {
        std::unordered_set<std::string> new_files;
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                new_files.insert(entry.path().filename().string());
            }
        }
        
        for (const auto& file : new_files) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "[+] File added: " << file << std::endl;
            }
        }
        
        for (const auto& file : current_files) {
            if (new_files.find(file) == new_files.end()) {
                std::cout << "[-] File removed: " << file << std::endl;
            }
        }
        
        current_files = std::move(new_files);
    }

public:
    DirectoryWatcher(const std::string& path) : watch_path(path) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Invalid directory path");
        }
        scan_directory();
        std::cout << "Watching directory: " << watch_path << std::endl;
    }

    void start_monitoring(int interval_seconds = 2) {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            try {
                scan_directory();
            } catch (const std::exception& e) {
                std::cerr << "Error scanning directory: " << e.what() << std::endl;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        DirectoryWatcher watcher(argv[1]);
        watcher.start_monitoring();
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize watcher: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}