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

    SimpleFileWatcher(const fs::path& directory, FileChangeCallback callback)
        : watch_directory(directory), change_callback(callback), running(false) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            throw std::runtime_error("Directory does not exist or is not a directory.");
        }
        build_file_map();
    }

    void start() {
        running = true;
        watcher_thread = std::thread(&SimpleFileWatcher::watch_loop, this);
    }

    void stop() {
        running = false;
        if (watcher_thread.joinable()) {
            watcher_thread.join();
        }
    }

    ~SimpleFileWatcher() {
        stop();
    }

private:
    fs::path watch_directory;
    FileChangeCallback change_callback;
    std::thread watcher_thread;
    bool running;
    std::unordered_map<std::string, fs::file_time_type> file_modification_map;

    void build_file_map() {
        file_modification_map.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                auto path = entry.path();
                auto mod_time = fs::last_write_time(path);
                file_modification_map[path.string()] = mod_time;
            }
        }
    }

    void watch_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            std::unordered_map<std::string, fs::file_time_type> current_map;
            for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
                if (entry.is_regular_file()) {
                    auto path = entry.path();
                    auto mod_time = fs::last_write_time(path);
                    current_map[path.string()] = mod_time;
                }
            }

            for (const auto& [path, current_time] : current_map) {
                auto it = file_modification_map.find(path);
                if (it == file_modification_map.end()) {
                    change_callback(path, "CREATED");
                } else if (it->second != current_time) {
                    change_callback(path, "MODIFIED");
                }
            }

            for (const auto& [path, old_time] : file_modification_map) {
                if (current_map.find(path) == current_map.end()) {
                    change_callback(path, "DELETED");
                }
            }

            file_modification_map = std::move(current_map);
        }
    }
};

void example_callback(const fs::path& file_path, const std::string& change_type) {
    std::cout << "File: " << file_path.filename() << " - Action: " << change_type << std::endl;
}

int main() {
    try {
        SimpleFileWatcher watcher(fs::current_path(), example_callback);
        std::cout << "Watching directory: " << fs::current_path() << std::endl;
        std::cout << "Press Enter to stop watching..." << std::endl;
        watcher.start();
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

            if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
                std::cerr << "Path is not accessible or not a directory." << std::endl;
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

int main() {
    std::string path = ".";
    FileSystemWatcher watcher(path);

    std::thread watch_thread([&watcher]() {
        watcher.start_watching();
    });

    std::this_thread::sleep_for(std::chrono::seconds(10));
    watcher.stop_watching();
    watch_thread.join();

    return 0;
}