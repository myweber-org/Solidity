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
        refresh_file_set();
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
    std::unordered_set<std::string> current_files;
    std::thread monitor_thread;
    bool running;

    void refresh_file_set() {
        current_files.clear();
        for (const auto& entry : fs::directory_iterator(watch_directory)) {
            current_files.insert(entry.path().filename().string());
        }
    }

    void monitor_loop() {
        while (running) {
            auto previous_files = current_files;
            refresh_file_set();

            for (const auto& file : current_files) {
                if (previous_files.find(file) == previous_files.end()) {
                    notify_callback(watch_directory / file, "created");
                }
            }

            for (const auto& file : previous_files) {
                if (current_files.find(file) == current_files.end()) {
                    notify_callback(watch_directory / file, "deleted");
                }
            }

            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
};

void example_callback(const fs::path& file_path, const std::string& action) {
    std::cout << "File: " << file_path << " Action: " << action << std::endl;
}

int main() {
    try {
        FileSystemWatcher watcher("./test_dir", example_callback);
        watcher.start();

        std::cout << "Watching directory './test_dir'. Press Enter to stop..." << std::endl;
        std::cin.get();

        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}