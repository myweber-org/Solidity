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

    std::unordered_set<std::string> get_files_in_directory() {
        std::unordered_set<std::string> files;
        for (const auto& entry : fs::directory_iterator(path_to_watch)) {
            if (entry.is_regular_file()) {
                files.insert(entry.path().filename().string());
            }
        }
        return files;
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        current_files = get_files_in_directory();
        std::cout << "Watching directory: " << fs::absolute(path_to_watch) << std::endl;
    }

    void start_monitoring(int interval_seconds = 2) {
        std::cout << "Monitoring started. Press Ctrl+C to stop." << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            auto new_files = get_files_in_directory();

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

            current_files = new_files;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start_monitoring();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}#include <iostream>
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
            throw std::runtime_error("Watch path does not exist or is not a directory");
        }
        buildFileSnapshot();
    }

    ~SimpleFileWatcher() {
        stop();
    }

    void start(int interval_ms = 1000) {
        running_ = true;
        watcher_thread_ = std::thread([this, interval_ms]() {
            while (running_) {
                checkForChanges();
                std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
            }
        });
    }

    void stop() {
        running_ = false;
        if (watcher_thread_.joinable()) {
            watcher_thread_.join();
        }
    }

private:
    void buildFileSnapshot() {
        file_snapshot_.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (entry.is_regular_file()) {
                auto last_write = fs::last_write_time(entry);
                file_snapshot_[entry.path()] = last_write;
            }
        }
    }

    void checkForChanges() {
        std::unordered_map<fs::path, fs::file_time_type> current_snapshot;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (entry.is_regular_file()) {
                auto path = entry.path();
                auto last_write = fs::last_write_time(entry);
                current_snapshot[path] = last_write;

                auto it = file_snapshot_.find(path);
                if (it == file_snapshot_.end()) {
                    if (callback_) callback_(path, "created");
                } else if (it->second != last_write) {
                    if (callback_) callback_(path, "modified");
                }
            }
        }

        for (const auto& [path, _] : file_snapshot_) {
            if (current_snapshot.find(path) == current_snapshot.end()) {
                if (callback_) callback_(path, "deleted");
            }
        }

        file_snapshot_.swap(current_snapshot);
    }

    fs::path watch_path_;
    FileChangeCallback callback_;
    std::unordered_map<fs::path, fs::file_time_type> file_snapshot_;
    std::thread watcher_thread_;
    std::atomic<bool> running_;
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

        watcher.start(2000);

        std::cin.get();
        watcher.stop();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}