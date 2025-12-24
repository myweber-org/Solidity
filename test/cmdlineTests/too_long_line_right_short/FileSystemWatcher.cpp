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

    std::unordered_set<std::string> get_files_in_directory() {
        std::unordered_set<std::string> files;
        for (const auto& entry : fs::directory_iterator(path_to_watch)) {
            if (fs::is_regular_file(entry.status())) {
                files.insert(entry.path().filename().string());
            }
        }
        return files;
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        current_files = get_files_in_directory();
        std::cout << "Watching directory: " << fs::absolute(path_to_watch) << std::endl;
    }

    void start_monitoring(int interval_seconds = 2) {
        std::cout << "Monitoring started. Checking every " << interval_seconds << " seconds." << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            auto new_files = get_files_in_directory();

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
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
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