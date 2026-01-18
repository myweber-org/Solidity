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
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher(const fs::path& watch_path, Callback callback)
        : watch_path_(watch_path), callback_(callback), running_(false) {
        if (!fs::exists(watch_path_)) {
            throw std::runtime_error("Watch path does not exist");
        }
        if (!fs::is_directory(watch_path_)) {
            throw std::runtime_error("Watch path must be a directory");
        }
        scan_files();
    }

    ~FileSystemWatcher() {
        stop();
    }

    void start() {
        running_ = true;
        monitor_thread_ = std::thread(&FileSystemWatcher::monitor, this);
    }

    void stop() {
        running_ = false;
        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
    }

private:
    void scan_files() {
        file_states_.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (entry.is_regular_file()) {
                auto last_write = entry.last_write_time();
                file_states_[entry.path()] = last_write;
            }
        }
    }

    void monitor() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            std::unordered_map<fs::path, fs::file_time_type> current_states;

            for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
                if (entry.is_regular_file()) {
                    auto path = entry.path();
                    auto last_write = entry.last_write_time();
                    current_states[path] = last_write;

                    auto it = file_states_.find(path);
                    if (it == file_states_.end()) {
                        if (callback_) {
                            callback_(path, "created");
                        }
                    } else if (it->second != last_write) {
                        if (callback_) {
                            callback_(path, "modified");
                        }
                    }
                }
            }

            for (const auto& [path, _] : file_states_) {
                if (current_states.find(path) == current_states.end()) {
                    if (callback_) {
                        callback_(path, "deleted");
                    }
                }
            }

            file_states_.swap(current_states);
        }
    }

    fs::path watch_path_;
    Callback callback_;
    std::unordered_map<fs::path, fs::file_time_type> file_states_;
    std::thread monitor_thread_;
    std::atomic<bool> running_;
};

void example_usage() {
    try {
        FileSystemWatcher watcher("./watch_dir", [](const fs::path& path, const std::string& action) {
            std::cout << "File: " << path << " Action: " << action << std::endl;
        });

        watcher.start();
        std::cout << "Watching directory. Press Enter to stop..." << std::endl;
        std::cin.get();
        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}