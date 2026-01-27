
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>
#include <functional>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using FileChangeCallback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher(const fs::path& watch_path, FileChangeCallback callback)
        : watch_path_(watch_path), callback_(callback), running_(false) {
        if (!fs::exists(watch_path_) || !fs::is_directory(watch_path_)) {
            throw std::runtime_error("Invalid watch directory");
        }
        initialize_file_map();
    }

    ~FileSystemWatcher() {
        stop();
    }

    void start() {
        running_ = true;
        monitor_thread_ = std::thread(&FileSystemWatcher::monitor_loop, this);
    }

    void stop() {
        running_ = false;
        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
    }

private:
    void initialize_file_map() {
        file_map_.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (entry.is_regular_file()) {
                file_map_[entry.path()] = entry.last_write_time();
            }
        }
    }

    void monitor_loop() {
        while (running_) {
            std::this_thread::sleep_for(check_interval_);

            auto current_files = file_map_;
            bool changes_detected = false;

            for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
                if (!entry.is_regular_file()) continue;

                const auto& path = entry.path();
                auto current_write_time = entry.last_write_time();

                if (file_map_.find(path) == file_map_.end()) {
                    file_map_[path] = current_write_time;
                    if (callback_) callback_(path, "created");
                    changes_detected = true;
                } else if (file_map_[path] != current_write_time) {
                    file_map_[path] = current_write_time;
                    if (callback_) callback_(path, "modified");
                    changes_detected = true;
                }
                current_files.erase(path);
            }

            for (const auto& [path, _] : current_files) {
                file_map_.erase(path);
                if (callback_) callback_(path, "deleted");
                changes_detected = true;
            }

            if (changes_detected) {
                std::cout << "File system changes detected in: " << watch_path_ << std::endl;
            }
        }
    }

    fs::path watch_path_;
    FileChangeCallback callback_;
    std::unordered_map<fs::path, fs::file_time_type> file_map_;
    std::thread monitor_thread_;
    std::atomic<bool> running_;
    std::chrono::milliseconds check_interval_{1000};
};

void example_callback(const fs::path& file_path, const std::string& change_type) {
    std::cout << "File: " << file_path.filename() << " - Action: " << change_type << std::endl;
}

int main() {
    try {
        fs::path watch_directory = fs::current_path() / "watch_folder";
        fs::create_directories(watch_directory);

        FileSystemWatcher watcher(watch_directory, example_callback);
        watcher.start();

        std::cout << "Watching directory: " << watch_directory << std::endl;
        std::cout << "Press Enter to stop watching..." << std::endl;
        std::cin.get();

        watcher.stop();
        fs::remove_all(watch_directory);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}