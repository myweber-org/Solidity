
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>
#include <ctime>

namespace fs = std::filesystem;

class PollingFileWatcher {
private:
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps;
    bool running;

    void scan_directory() {
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                auto current_time = fs::last_write_time(entry.path());
                std::string file_path = entry.path().string();

                if (file_timestamps.find(file_path) == file_timestamps.end()) {
                    file_timestamps[file_path] = current_time;
                    std::cout << "[NEW] File detected: " << file_path << std::endl;
                } else {
                    if (file_timestamps[file_path] != current_time) {
                        file_timestamps[file_path] = current_time;
                        std::cout << "[MODIFIED] File changed: " << file_path << std::endl;
                    }
                }
            }
        }

        std::vector<std::string> to_remove;
        for (const auto& [file_path, timestamp] : file_timestamps) {
            if (!fs::exists(file_path)) {
                to_remove.push_back(file_path);
            }
        }

        for (const auto& file_path : to_remove) {
            file_timestamps.erase(file_path);
            std::cout << "[DELETED] File removed: " << file_path << std::endl;
        }
    }

public:
    PollingFileWatcher(const std::string& path) : watch_path(path), running(false) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Invalid directory path: " + path);
        }
    }

    void start(int interval_seconds = 2) {
        running = true;
        std::cout << "Starting file watcher on: " << watch_path.string() << std::endl;
        std::cout << "Polling interval: " << interval_seconds << " seconds" << std::endl;

        while (running) {
            try {
                scan_directory();
            } catch (const std::exception& e) {
                std::cerr << "Error scanning directory: " << e.what() << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
        }
    }

    void stop() {
        running = false;
        std::cout << "File watcher stopped." << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        PollingFileWatcher watcher(argv[1]);
        watcher.start();

        std::cout << "Press Enter to stop watching..." << std::endl;
        std::cin.get();

        watcher.stop();
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

    std::unordered_set<std::string> getDirectoryContents() {
        std::unordered_set<std::string> files;
        if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                files.insert(entry.path().filename().string());
            }
        }
        return files;
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        current_files = getDirectoryContents();
    }

    void startWatching(int interval_seconds = 2) {
        running = true;
        std::cout << "Starting to watch: " << path_to_wick << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            auto new_files = getDirectoryContents();

            // Check for added files
            for (const auto& file : new_files) {
                if (current_files.find(file) == current_files.end()) {
                    std::cout << "[ADDED] " << file << std::endl;
                }
            }

            // Check for removed files
            for (const auto& file : current_files) {
                if (new_files.find(file) == new_files.end()) {
                    std::cout << "[REMOVED] " << file << std::endl;
                }
            }

            current_files = new_files;
        }
    }

    void stopWatching() {
        running = false;
        std::cout << "Stopped watching." << std::endl;
    }
};

int main() {
    std::string path = ".";
    FileSystemWatcher watcher(path);

    // Run for 10 seconds then stop
    std::thread watch_thread([&watcher]() {
        watcher.startWatching(1);
    });

    std::this_thread::sleep_for(std::chrono::seconds(10));
    watcher.stopWatching();
    watch_thread.join();

    return 0;
}