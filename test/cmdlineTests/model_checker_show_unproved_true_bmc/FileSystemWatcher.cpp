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

    FileSystemWatcher(const fs::path& watch_path, Callback callback)
        : watch_path_(watch_path), callback_(callback), running_(false) {
        if (!fs::exists(watch_path_) || !fs::is_directory(watch_path_)) {
            throw std::runtime_error("Watch path does not exist or is not a directory");
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

    void start() {
        running_ = true;
        snapshot_ = take_snapshot();
        watcher_thread_ = std::thread(&FileSystemWatcher::watch_loop, this);
    }

    void stop() {
        running_ = false;
        if (watcher_thread_.joinable()) {
            watcher_thread_.join();
        }
    }

private:
    fs::path watch_path_;
    Callback callback_;
    bool running_;
    std::thread watcher_thread_;
    std::unordered_set<std::string> snapshot_;

    std::unordered_set<std::string> take_snapshot() {
        std::unordered_set<std::string> snapshot;
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            snapshot.insert(entry.path().string());
        }
        return snapshot;
    }

    void watch_loop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            auto current_snapshot = take_snapshot();

            // Detect new files
            for (const auto& path_str : current_snapshot) {
                if (snapshot_.find(path_str) == snapshot_.end()) {
                    callback_(path_str, "created");
                }
            }

            // Detect deleted files
            for (const auto& path_str : snapshot_) {
                if (current_snapshot.find(path_str) == current_snapshot.end()) {
                    callback_(path_str, "deleted");
                }
            }

            snapshot_ = std::move(current_snapshot);
        }
    }
};

void example_callback(const fs::path& path, const std::string& action) {
    std::cout << "File " << path << " was " << action << std::endl;
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