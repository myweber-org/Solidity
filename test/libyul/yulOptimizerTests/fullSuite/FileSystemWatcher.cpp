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
    }

    ~FileSystemWatcher() {
        stop();
    }

    void start() {
        running = true;
        snapshot_current_state();
        watcher_thread = std::thread(&FileSystemWatcher::watch_loop, this);
    }

    void stop() {
        running = false;
        if (watcher_thread.joinable()) {
            watcher_thread.join();
        }
    }

private:
    fs::path watch_directory;
    Callback notify_callback;
    std::unordered_set<std::string> file_snapshot;
    std::thread watcher_thread;
    bool running;

    void snapshot_current_state() {
        file_snapshot.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                file_snapshot.insert(entry.path().string());
            }
        }
    }

    void watch_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            std::unordered_set<std::string> current_files;
            for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
                if (entry.is_regular_file()) {
                    current_files.insert(entry.path().string());
                }
            }

            for (const auto& file : current_files) {
                if (file_snapshot.find(file) == file_snapshot.end()) {
                    notify_callback(file, "created");
                }
            }

            for (const auto& file : file_snapshot) {
                if (current_files.find(file) == current_files.end()) {
                    notify_callback(file, "deleted");
                }
            }

            file_snapshot = std::move(current_files);
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher("./logs", [](const fs::path& path, const std::string& action) {
            std::cout << "File " << path << " was " << action << ".\n";
        });

        watcher.start();
        std::cout << "Watching directory './logs'. Press Enter to stop...\n";
        std::cin.get();
        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}