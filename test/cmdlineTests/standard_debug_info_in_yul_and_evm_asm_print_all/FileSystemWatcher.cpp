#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <unordered_set>
#include <atomic>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const fs::path& path) : watch_path(path), running(false) {
        if (!fs::exists(path) || !fs::is_directory(path)) {
            throw std::runtime_error("Invalid directory path");
        }
        update_snapshot();
    }

    void start() {
        running = true;
        watcher_thread = std::thread(&FileSystemWatcher::watch_loop, this);
    }

    void stop() {
        running = false;
        if (watcher_thread.joinable()) {
            watcher_thread.join();
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

private:
    fs::path watch_path;
    std::atomic<bool> running;
    std::thread watcher_thread;
    std::unordered_set<std::string> file_snapshot;

    void update_snapshot() {
        file_snapshot.clear();
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                file_snapshot.insert(entry.path().filename().string());
            }
        }
    }

    void watch_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            std::unordered_set<std::string> current_files;
            for (const auto& entry : fs::directory_iterator(watch_path)) {
                if (entry.is_regular_file()) {
                    current_files.insert(entry.path().filename().string());
                }
            }

            for (const auto& file : current_files) {
                if (file_snapshot.find(file) == file_snapshot.end()) {
                    std::cout << "New file detected: " << file << std::endl;
                }
            }

            for (const auto& file : file_snapshot) {
                if (current_files.find(file) == current_files.end()) {
                    std::cout << "File removed: " << file << std::endl;
                }
            }

            file_snapshot = std::move(current_files);
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start();

        std::cout << "Watching current directory. Press Enter to stop..." << std::endl;
        std::cin.get();

        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const std::string& path) : watch_path(path) {
        if (fs::exists(path) && fs::is_directory(path)) {
            refresh_snapshot();
        } else {
            std::cerr << "Path does not exist or is not a directory: " << path << std::endl;
        }
    }

    void start_watching(int interval_seconds = 1) {
        std::cout << "Watching directory: " << watch_path << std::endl;
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

    void stop_watching() {
        running = false;
    }

private:
    std::string watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_snapshot;
    bool running = true;

    void refresh_snapshot() {
        file_snapshot.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_snapshot[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void check_for_changes() {
        auto current_state = file_snapshot;
        refresh_snapshot();

        for (const auto& [path, old_time] : current_state) {
            auto it = file_snapshot.find(path);
            if (it == file_snapshot.end()) {
                std::cout << "File deleted: " << path << std::endl;
            } else if (it->second != old_time) {
                std::cout << "File modified: " << path << std::endl;
            }
        }

        for (const auto& [path, new_time] : file_snapshot) {
            if (current_state.find(path) == current_state.end()) {
                std::cout << "File created: " << path << std::endl;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    FileSystemWatcher watcher(argv[1]);
    watcher.start_watching(2);

    return 0;
}