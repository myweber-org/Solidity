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

    void populate_file_set() {
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
        if (!fs::exists(path_to_watch)) {
            throw std::runtime_error("Path does not exist: " + path);
        }
        populate_file_set();
        std::cout << "Watching directory: " << fs::absolute(path_to_watch) << std::endl;
    }

    void start_monitoring(int interval_seconds = 2) {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            std::unordered_set<std::string> new_files;
            bool changes_detected = false;

            if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
                for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                    if (fs::is_regular_file(entry.status())) {
                        std::string filename = entry.path().filename().string();
                        new_files.insert(filename);

                        if (current_files.find(filename) == current_files.end()) {
                            std::cout << "File added: " << filename << std::endl;
                            changes_detected = true;
                        }
                    }
                }

                for (const auto& old_file : current_files) {
                    if (new_files.find(old_file) == new_files.end()) {
                        std::cout << "File removed: " << old_file << std::endl;
                        changes_detected = true;
                    }
                }
            }

            if (changes_detected) {
                current_files = std::move(new_files);
            }
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