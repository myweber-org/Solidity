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
        if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                current_files.insert(entry.path().filename().string());
            }
        }
    }

public:
    explicit FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        populate_file_set();
    }

    void start_watching(int interval_seconds = 2) {
        running = true;
        std::cout << "Started watching: " << path_to_wwatch.string() << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            if (!fs::exists(path_to_watch)) {
                std::cerr << "Directory no longer exists: " << path_to_watch.string() << std::endl;
                break;
            }

            std::unordered_set<std::string> new_files;
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                new_files.insert(entry.path().filename().string());
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
    }

    void stop_watching() {
        running = false;
        std::cout << "Stopped watching." << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    FileSystemWatcher watcher(argv[1]);
    watcher.start_watching();

    return 0;
}