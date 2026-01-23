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
        if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                if (fs::is_regular_file(entry.status())) {
                    files.insert(entry.path().filename().string());
                }
            }
        }
        return files;
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            current_files = get_files_in_directory();
            std::cout << "Watching directory: " << fs::absolute(path_to_watch) << std::endl;
            std::cout << "Initial file count: " << current_files.size() << std::endl;
        } else {
            std::cerr << "Path does not exist or is not a directory: " << path << std::endl;
        }
    }

    void start_monitoring(int interval_seconds = 2) {
        if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
            return;
        }

        std::cout << "Starting monitoring. Press Ctrl+C to stop." << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            auto new_files = get_files_in_directory();

            // Check for added files
            for (const auto& file : new_files) {
                if (current_files.find(file) == current_files.end()) {
                    std::cout << "[+] File added: " << file << std::endl;
                }
            }

            // Check for removed files
            for (const auto& file : current_files) {
                if (new_files.find(file) == new_files.end()) {
                    std::cout << "[-] File removed: " << file << std::endl;
                }
            }

            current_files = new_files;
        }
    }
};

int main(int argc, char* argv[]) {
    std::string path = ".";
    if (argc > 1) {
        path = argv[1];
    }

    FileSystemWatcher watcher(path);
    watcher.start_monitoring();

    return 0;
}
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>
#include <functional>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    using FileChangeCallback = std::function<void(const fs::path&, const std::string&)>;

    SimpleFileWatcher(const fs::path& watch_path, FileChangeCallback callback)
        : watch_directory(watch_path), change_callback(callback), running(false) {
        if (!fs::exists(watch_directory) || !fs::is_directory(watch_directory)) {
            throw std::runtime_error("Invalid watch directory path");
        }
        initialize_file_map();
    }

    ~SimpleFileWatcher() {
        stop();
    }

    void start(int interval_ms = 1000) {
        running = true;
        watcher_thread = std::thread(&SimpleFileWatcher::watch_loop, this, interval_ms);
    }

    void stop() {
        running = false;
        if (watcher_thread.joinable()) {
            watcher_thread.join();
        }
    }

private:
    void initialize_file_map() {
        file_state_map.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.path())) {
                file_state_map[entry.path()] = fs::last_write_time(entry.path());
            }
        }
    }

    void watch_loop(int interval_ms) {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
            check_for_changes();
        }
    }

    void check_for_changes() {
        std::unordered_map<fs::path, fs::file_time_type> current_state;

        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.path())) {
                current_state[entry.path()] = fs::last_write_time(entry.path());
            }
        }

        for (const auto& [path, current_time] : current_state) {
            auto it = file_state_map.find(path);
            if (it == file_state_map.end()) {
                file_state_map[path] = current_time;
                if (change_callback) {
                    change_callback(path, "CREATED");
                }
            } else if (it->second != current_time) {
                file_state_map[path] = current_time;
                if (change_callback) {
                    change_callback(path, "MODIFIED");
                }
            }
        }

        std::vector<fs::path> removed_files;
        for (const auto& [path, old_time] : file_state_map) {
            if (current_state.find(path) == current_state.end()) {
                removed_files.push_back(path);
            }
        }

        for (const auto& path : removed_files) {
            file_state_map.erase(path);
            if (change_callback) {
                change_callback(path, "DELETED");
            }
        }
    }

    fs::path watch_directory;
    FileChangeCallback change_callback;
    std::unordered_map<fs::path, fs::file_time_type> file_state_map;
    std::thread watcher_thread;
    std::atomic<bool> running;
};

void example_callback(const fs::path& file_path, const std::string& change_type) {
    std::cout << "File: " << file_path << " - Change: " << change_type << std::endl;
}

int main() {
    try {
        fs::path current_path = fs::current_path();
        SimpleFileWatcher watcher(current_path, example_callback);

        std::cout << "Watching directory: " << current_path << std::endl;
        std::cout << "Press Enter to stop watching..." << std::endl;

        watcher.start(2000);

        std::cin.get();
        watcher.stop();

        std::cout << "File watching stopped." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}