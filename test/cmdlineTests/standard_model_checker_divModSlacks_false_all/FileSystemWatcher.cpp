#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& directory) : watch_path(directory) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        populate_file_map();
    }

    void start_watching(int interval_seconds = 1) {
        std::cout << "Watching directory: " << watch_path.string() << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps;

    void populate_file_map() {
        file_timestamps.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.status())) {
                file_timestamps[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void check_for_changes() {
        auto it = file_timestamps.begin();
        while (it != file_timestamps.end()) {
            if (!fs::exists(it->first)) {
                std::cout << "File deleted: " << it->first << std::endl;
                it = file_timestamps.erase(it);
            } else {
                ++it;
            }
        }

        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.status())) {
                std::string file_path = entry.path().string();
                auto current_time = fs::last_write_time(entry);

                if (file_timestamps.find(file_path) == file_timestamps.end()) {
                    std::cout << "New file created: " << file_path << std::endl;
                    file_timestamps[file_path] = current_time;
                } else if (file_timestamps[file_path] != current_time) {
                    std::cout << "File modified: " << file_path << std::endl;
                    file_timestamps[file_path] = current_time;
                }
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start_watching();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
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
        if (!fs::is_directory(watch_path_)) {
            throw std::runtime_error("Watch path must be a directory");
        }
    }

    void start(int interval_ms = 1000) {
        running_ = true;
        scan_current_state();
        watcher_thread_ = std::thread([this, interval_ms]() {
            while (running_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
                check_for_changes();
            }
        });
    }

    void stop() {
        running_ = false;
        if (watcher_thread_.joinable()) {
            watcher_thread_.join();
        }
    }

    ~SimpleFileWatcher() {
        stop();
    }

private:
    void scan_current_state() {
        file_states_.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (entry.is_regular_file()) {
                auto last_write = fs::last_write_time(entry.path());
                file_states_[entry.path()] = last_write;
            }
        }
    }

    void check_for_changes() {
        std::unordered_map<fs::path, fs::file_time_type> current_states;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (entry.is_regular_file()) {
                auto path = entry.path();
                auto last_write = fs::last_write_time(path);
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

    fs::path watch_path_;
    FileChangeCallback callback_;
    std::unordered_map<fs::path, fs::file_time_type> file_states_;
    std::thread watcher_thread_;
    std::atomic<bool> running_;
};

void example_callback(const fs::path& path, const std::string& change_type) {
    std::cout << "File " << path << " was " << change_type << std::endl;
}

int main() {
    try {
        SimpleFileWatcher watcher(".", example_callback);
        watcher.start(2000);

        std::cout << "Watching current directory. Press Enter to stop..." << std::endl;
        std::cin.get();

        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}