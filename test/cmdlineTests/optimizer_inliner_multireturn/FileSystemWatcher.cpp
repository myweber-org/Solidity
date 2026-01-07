
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
    bool running = false;

    void populate_file_set() {
        current_files.clear();
        for (const auto& entry : fs::directory_iterator(path_to_watch)) {
            current_files.insert(entry.path().filename().string());
        }
    }

public:
    explicit FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
            throw std::invalid_argument("Path does not exist or is not a directory");
        }
        populate_file_set();
    }

    void start(int interval_seconds = 1) {
        running = true;
        std::cout << "Watching directory: " << path_to_watch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            auto temp_files = current_files;
            populate_file_set();

            // Check for new files
            for (const auto& file : current_files) {
                if (temp_files.find(file) == temp_files.end()) {
                    std::cout << "[+] New file detected: " << file << std::endl;
                }
            }

            // Check for deleted files
            for (const auto& file : temp_files) {
                if (current_files.find(file) == current_files.end()) {
                    std::cout << "[-] File deleted: " << file << std::endl;
                }
            }
        }
    }

    void stop() {
        running = false;
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start(2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}