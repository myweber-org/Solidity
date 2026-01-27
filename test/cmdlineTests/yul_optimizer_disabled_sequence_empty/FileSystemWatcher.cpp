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
            throw std::runtime_error("Invalid directory path provided");
        }
        initialize_file_set();
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
    void initialize_file_set() {
        current_files.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.status())) {
                current_files.insert(fs::canonical(entry.path()));
            }
        }
    }

    void monitor_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            auto previous_files = current_files;
            initialize_file_set();

            detect_file_changes(previous_files);
        }
    }

    void detect_file_changes(const std::unordered_set<fs::path>& previous_set) {
        for (const auto& file : current_files) {
            if (previous_set.find(file) == previous_set.end()) {
                notify_callback(file, "created");
            }
        }

        for (const auto& file : previous_set) {
            if (current_files.find(file) == current_files.end()) {
                notify_callback(file, "deleted");
            }
        }
    }

    fs::path watch_directory;
    Callback notify_callback;
    std::unordered_set<fs::path> current_files;
    std::thread monitor_thread;
    std::atomic<bool> running;
};

void example_usage() {
    try {
        FileSystemWatcher watcher(fs::current_path(), [](const fs::path& path, const std::string& action) {
            std::cout << "File " << path.filename() << " has been " << action << std::endl;
        });

        watcher.start();
        std::cout << "Watching directory: " << fs::current_path() << std::endl;
        std::cout << "Press Enter to stop monitoring..." << std::endl;
        std::cin.get();
        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}