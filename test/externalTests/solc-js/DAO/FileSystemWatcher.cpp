
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
        : watch_path_(watch_path), callback_(callback), running_(false) {
        if (!fs::exists(watch_path_) || !fs::is_directory(watch_path_)) {
            throw std::runtime_error("Invalid watch directory");
        }
        scanExistingFiles();
    }

    ~SimpleFileWatcher() {
        stop();
    }

    void start() {
        running_ = true;
        monitor_thread_ = std::thread(&SimpleFileWatcher::monitorLoop, this);
    }

    void stop() {
        running_ = false;
        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
    }

private:
    void scanExistingFiles() {
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (fs::is_regular_file(entry.status())) {
                file_states_[entry.path()] = fs::last_write_time(entry);
            }
        }
    }

    void monitorLoop() {
        while (running_) {
            checkForChanges();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void checkForChanges() {
        std::unordered_map<fs::path, fs::file_time_type> current_states;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (fs::is_regular_file(entry.status())) {
                current_states[entry.path()] = fs::last_write_time(entry);
            }
        }

        for (const auto& [path, mtime] : current_states) {
            auto it = file_states_.find(path);
            if (it == file_states_.end()) {
                if (callback_) callback_(path, "created");
            } else if (it->second != mtime) {
                if (callback_) callback_(path, "modified");
            }
        }

        for (const auto& [path, mtime] : file_states_) {
            if (current_states.find(path) == current_states.end()) {
                if (callback_) callback_(path, "deleted");
            }
        }

        file_states_.swap(current_states);
    }

    fs::path watch_path_;
    FileChangeCallback callback_;
    std::unordered_map<fs::path, fs::file_time_type> file_states_;
    std::thread monitor_thread_;
    bool running_;
};

void exampleCallback(const fs::path& file_path, const std::string& change_type) {
    std::cout << "File: " << file_path.filename() << " - Action: " << change_type << std::endl;
}

int main() {
    try {
        fs::path watch_dir = ".";
        SimpleFileWatcher watcher(watch_dir, exampleCallback);
        
        std::cout << "Watching directory: " << fs::absolute(watch_dir) << std::endl;
        std::cout << "Press Enter to stop watching..." << std::endl;
        
        watcher.start();
        std::cin.get();
        watcher.stop();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}