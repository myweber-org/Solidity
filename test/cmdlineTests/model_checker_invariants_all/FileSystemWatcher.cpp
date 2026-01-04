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
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        populate_file_set();
    }

    void start_monitoring(int interval_seconds = 2) {
        running = true;
        std::cout << "Starting monitoring of: " << path_to_watch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            auto previous_files = current_files;
            populate_file_set();

            // Detect new files
            for (const auto& file : current_files) {
                if (previous_files.find(file) == previous_files.end()) {
                    std::cout << "[+] New file detected: " << file << std::endl;
                }
            }

            // Detect deleted files
            for (const auto& file : previous_files) {
                if (current_files.find(file) == current_files.end()) {
                    std::cout << "[-] File deleted: " << file << std::endl;
                }
            }
        }
    }

    void stop_monitoring() {
        running = false;
        std::cout << "Monitoring stopped." << std::endl;
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