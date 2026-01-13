#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>
#include <functional>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher(const fs::path& directory, Callback callback)
        : watch_directory(directory), notify_callback(callback), running(false) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        scan_existing_files();
    }

    ~FileSystemWatcher() {
        stop();
    }

    void start() {
        running = true;
        monitor_thread = std::thread(&FileSystemWatcher::monitor_loop, this);
    }

    void stop() {
        running = false;
        if (monitor_thread.joinable()) {
            monitor_thread.join();
        }
    }

private:
    fs::path watch_directory;
    Callback notify_callback;
    std::unordered_set<std::string> known_files;
    std::thread monitor_thread;
    bool running;

    void scan_existing_files() {
        known_files.clear();
        for (const auto& entry : fs::directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                known_files.insert(entry.path().filename().string());
            }
        }
    }

    void monitor_loop() {
        while (running) {
            auto current_files = get_current_file_list();
            detect_changes(current_files);
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }

    std::unordered_set<std::string> get_current_file_list() {
        std::unordered_set<std::string> current;
        for (const auto& entry : fs::directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                current.insert(entry.path().filename().string());
            }
        }
        return current;
    }

    void detect_changes(const std::unordered_set<std::string>& current) {
        for (const auto& file : current) {
            if (known_files.find(file) == known_files.end()) {
                notify_callback(watch_directory / file, "created");
            }
        }

        for (const auto& old_file : known_files) {
            if (current.find(old_file) == current.end()) {
                notify_callback(watch_directory / old_file, "deleted");
            }
        }

        known_files = current;
    }
};

void example_callback(const fs::path& file_path, const std::string& action) {
    std::cout << "File: " << file_path.filename() << " Action: " << action << std::endl;
}

int main() {
    try {
        FileSystemWatcher watcher("./test_dir", example_callback);
        watcher.start();

        std::cout << "Watching directory. Press Enter to stop..." << std::endl;
        std::cin.get();

        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}