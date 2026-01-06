
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
        if (!fs::exists(watch_path_)) {
            throw std::runtime_error("Watch path does not exist");
        }
        scanCurrentState();
    }

    ~SimpleFileWatcher() {
        stop();
    }

    void start() {
        running_ = true;
        watcher_thread_ = std::thread(&SimpleFileWatcher::watchLoop, this);
    }

    void stop() {
        running_ = false;
        if (watcher_thread_.joinable()) {
            watcher_thread_.join();
        }
    }

private:
    void scanCurrentState() {
        file_states_.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (entry.is_regular_file()) {
                auto last_write = fs::last_write_time(entry.path());
                file_states_[entry.path()] = last_write;
            }
        }
    }

    void watchLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            std::unordered_map<fs::path, fs::file_time_type> current_states;

            for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
                if (entry.is_regular_file()) {
                    current_states[entry.path()] = fs::last_write_time(entry.path());
                }
            }

            for (const auto& [path, current_time] : current_states) {
                auto it = file_states_.find(path);
                if (it == file_states_.end()) {
                    if (callback_) {
                        callback_(path, "created");
                    }
                } else if (it->second != current_time) {
                    if (callback_) {
                        callback_(path, "modified");
                    }
                }
            }

            for (const auto& [path, old_time] : file_states_) {
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
    FileChangeCallback callback_;
    std::unordered_map<fs::path, fs::file_time_type> file_states_;
    std::thread watcher_thread_;
    std::atomic<bool> running_;
};

void exampleCallback(const fs::path& file_path, const std::string& change_type) {
    std::cout << "File: " << file_path.string() << " - Action: " << change_type << std::endl;
}

int main() {
    try {
        SimpleFileWatcher watcher(".", exampleCallback);
        watcher.start();

        std::cout << "Watching current directory. Press Enter to stop..." << std::endl;
        std::cin.get();

        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}