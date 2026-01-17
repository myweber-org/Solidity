#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path path_to_watch;
    std::unordered_set<std::string> current_files;
    bool running = false;

    std::unordered_set<std::string> get_directory_contents() {
        std::unordered_set<std::string> files;
        if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                files.insert(entry.path().filename().string());
            }
        }
        return files;
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        current_files = get_directory_contents();
    }

    void start_watching(int interval_seconds = 2) {
        running = true;
        std::cout << "Watching directory: " << path_to_watch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            auto new_files = get_directory_contents();

            // Check for added files
            for (const auto& file : new_files) {
                if (current_files.find(file) == current_files.end()) {
                    std::cout << "[+] File added: " << file << std::endl;
                }
            }

            // Check for removed files
            for (const auto& file : current_files) {
                if (new_files.find(file) == new_files.end()) {
                    std::cout << "[-] File removed: " << file << std::endl;
                }
            }

            current_files = std::move(new_files);
        }
    }

    void stop_watching() {
        running = false;
    }
};

int main() {
    std::string watch_path = ".";
    FileSystemWatcher watcher(watch_path);

    std::thread watch_thread([&watcher]() {
        watcher.start_watching();
    });

    std::cout << "File system watcher started. Press Enter to stop..." << std::endl;
    std::cin.get();

    watcher.stop_watching();
    if (watch_thread.joinable()) {
        watch_thread.join();
    }

    std::cout << "Watcher stopped." << std::endl;
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
        snapshot_files();
    }

    void start() {
        running_ = true;
        watcher_thread_ = std::thread(&FileSystemWatcher::watch_loop, this);
    }

    void stop() {
        running_ = false;
        if (watcher_thread_.joinable()) {
            watcher_thread_.join();
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

private:
    void snapshot_files() {
        file_snapshot_.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (entry.is_regular_file()) {
                auto last_write = fs::last_write_time(entry);
                file_snapshot_[entry.path()] = last_write;
            }
        }
    }

    void watch_loop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            check_for_changes();
        }
    }

    void check_for_changes() {
        std::unordered_map<fs::path, fs::file_time_type> current_snapshot;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (entry.is_regular_file()) {
                auto path = entry.path();
                auto last_write = fs::last_write_time(entry);
                current_snapshot[path] = last_write;

                auto it = file_snapshot_.find(path);
                if (it == file_snapshot_.end()) {
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

        for (const auto& [path, _] : file_snapshot_) {
            if (current_snapshot.find(path) == current_snapshot.end()) {
                if (callback_) {
                    callback_(path, "deleted");
                }
            }
        }

        file_snapshot_.swap(current_snapshot);
    }

    fs::path watch_path_;
    Callback callback_;
    std::unordered_map<fs::path, fs::file_time_type> file_snapshot_;
    std::thread watcher_thread_;
    std::atomic<bool> running_;
};

void example_callback(const fs::path& path, const std::string& action) {
    std::cout << "File: " << path.filename() << " Action: " << action << std::endl;
}

int main() {
    try {
        fs::path watch_dir = fs::current_path() / "watch_folder";
        fs::create_directories(watch_dir);

        FileSystemWatcher watcher(watch_dir, example_callback);
        watcher.start();

        std::cout << "Watching directory: " << watch_dir << std::endl;
        std::cout << "Press Enter to stop..." << std::endl;
        std::cin.get();

        watcher.stop();
        fs::remove_all(watch_dir);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}