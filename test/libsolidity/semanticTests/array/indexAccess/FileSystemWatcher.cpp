#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path directory_to_watch;
    std::unordered_set<std::string> known_files;

    std::unordered_set<std::string> get_current_files() const {
        std::unordered_set<std::string> current_files;
        if (fs::exists(directory_to_watch) && fs::is_directory(directory_to_watch)) {
            for (const auto& entry : fs::directory_iterator(directory_to_watch)) {
                current_files.insert(entry.path().filename().string());
            }
        }
        return current_files;
    }

public:
    explicit FileSystemWatcher(const std::string& dir_path) : directory_to_watch(dir_path) {
        if (!fs::exists(directory_to_watch)) {
            fs::create_directories(directory_to_watch);
        }
        known_files = get_current_files();
        std::cout << "Watching directory: " << fs::absolute(directory_to_watch) << std::endl;
    }

    void check_for_changes() {
        auto current_files = get_current_files();

        for (const auto& file : current_files) {
            if (known_files.find(file) == known_files.end()) {
                std::cout << "[+] File added: " << file << std::endl;
            }
        }

        for (const auto& file : known_files) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "[-] File removed: " << file << std::endl;
            }
        }

        known_files = std::move(current_files);
    }

    void start_monitoring(int interval_seconds = 2) {
        std::cout << "Starting monitoring with interval " << interval_seconds << " seconds." << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }
};

int main(int argc, char* argv[]) {
    std::string path_to_watch = ".";
    if (argc > 1) {
        path_to_watch = argv[1];
    }

    try {
        FileSystemWatcher watcher(path_to_watch);
        watcher.start_monitoring();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}