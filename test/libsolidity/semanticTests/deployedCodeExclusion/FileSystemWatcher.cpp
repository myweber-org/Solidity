
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
        std::cout << "Started monitoring: " << path_to_watch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            auto previous_files = current_files;
            populate_file_set();

            // Detect new files
            for (const auto& filename : current_files) {
                if (previous_files.find(filename) == previous_files.end()) {
                    std::cout << "[NEW] " << filename << std::endl;
                }
            }

            // Detect deleted files
            for (const auto& filename : previous_files) {
                if (current_files.find(filename) == current_files.end()) {
                    std::cout << "[DELETED] " << filename << std::endl;
                }
            }
        }
    }

    void stop_monitoring() {
        running = false;
        std::cout << "Stopped monitoring." << std::endl;
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start_monitoring(3);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}#include <iostream>
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
        std::cout << "Watching directory: " << path_to_watch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            if (!fs::exists(path_to_watch)) {
                std::cout << "Directory deleted or moved. Stopping watch." << std::endl;
                stop();
                break;
            }

            std::unordered_set<std::string> new_files;
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                new_files.insert(entry.path().filename().string());
            }

            for (const auto& file : new_files) {
                if (current_files.find(file) == current_files.end()) {
                    std::cout << "File added: " << file << std::endl;
                }
            }

            for (const auto& file : current_files) {
                if (new_files.find(file) == new_files.end()) {
                    std::cout << "File removed: " << file << std::endl;
                }
            }

            current_files = std::move(new_files);
        }
    }

    void stop() {
        running = false;
    }
};

int main() {
    std::string watch_path = ".";
    FileSystemWatcher watcher(watch_path);
    watcher.start_watching();
    return 0;
}