#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <signal.h>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher(const fs::path& directory, Callback callback)
        : watch_dir(directory), callback_func(callback), running(false) {
        if (!fs::exists(watch_dir) || !fs::is_directory(watch_dir)) {
            throw std::runtime_error("Invalid directory path");
        }
        build_file_map();
    }

    ~FileSystemWatcher() {
        stop();
    }

    void start() {
        running = true;
        watcher_thread = std::thread(&FileSystemWatcher::watch_loop, this);
    }

    void stop() {
        running = false;
        if (watcher_thread.joinable()) {
            watcher_thread.join();
        }
    }

private:
    fs::path watch_dir;
    Callback callback_func;
    std::unordered_map<std::string, fs::file_time_type> file_map;
    std::thread watcher_thread;
    std::atomic<bool> running;

    void build_file_map() {
        file_map.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_dir)) {
            if (fs::is_regular_file(entry.status())) {
                file_map[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void watch_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            auto current_map = file_map;
            std::vector<std::string> created_files, modified_files, deleted_files;

            for (const auto& entry : fs::recursive_directory_iterator(watch_dir)) {
                if (!fs::is_regular_file(entry.status())) continue;

                std::string path_str = entry.path().string();
                auto current_time = fs::last_write_time(entry);

                if (current_map.find(path_str) == current_map.end()) {
                    created_files.push_back(path_str);
                } else if (current_map[path_str] != current_time) {
                    modified_files.push_back(path_str);
                }
                current_map.erase(path_str);
            }

            for (const auto& [path, _] : current_map) {
                deleted_files.push_back(path);
            }

            for (const auto& path : created_files) {
                callback_func(path, "CREATED");
                file_map[path] = fs::last_write_time(path);
            }
            for (const auto& path : modified_files) {
                callback_func(path, "MODIFIED");
                file_map[path] = fs::last_write_time(path);
            }
            for (const auto& path : deleted_files) {
                callback_func(path, "DELETED");
                file_map.erase(path);
            }
        }
    }
};

void signal_handler(int signal) {
    std::cout << "\nInterrupt signal received. Exiting...\n";
}

int main() {
    signal(SIGINT, signal_handler);

    try {
        FileSystemWatcher watcher("./test_watch", [](const fs::path& path, const std::string& action) {
            std::cout << "[" << action << "] " << path.filename().string() << std::endl;
        });

        std::cout << "Watching directory: ./test_watch\nPress Ctrl+C to stop.\n";
        watcher.start();

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }

        watcher.stop();
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
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const fs::path& path_to_watch) : path_to_watch_(fs::absolute(path_to_watch)) {
        if (!fs::exists(path_to_watch_) || !fs::is_directory(path_to_watch_)) {
            throw std::runtime_error("Provided path is not a valid directory.");
        }
        refresh_file_list();
    }

    void start_monitoring(int interval_seconds = 1) {
        std::cout << "Starting to monitor: " << path_to_watch_ << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path path_to_watch_;
    std::unordered_set<std::string> current_files_;

    void refresh_file_list() {
        current_files_.clear();
        for (const auto& entry : fs::directory_iterator(path_to_watch_)) {
            current_files_.insert(entry.path().filename().string());
        }
    }

    void check_for_changes() {
        auto previous_files = current_files_;
        refresh_file_list();

        for (const auto& file : current_files_) {
            if (previous_files.find(file) == previous_files.end()) {
                std::cout << "[+] File added: " << file << std::endl;
            }
        }

        for (const auto& file : previous_files) {
            if (current_files_.find(file) == current_files_.end()) {
                std::cout << "[-] File removed: " << file << std::endl;
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
        watcher.start_monitoring();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}