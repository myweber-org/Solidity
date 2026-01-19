#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <mutex>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher() : running_(false) {}

    void addWatchPath(const fs::path& path, Callback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (fs::exists(path) && fs::is_directory(path)) {
            watch_paths_[path] = {callback, getCurrentFileState(path)};
        }
    }

    void start() {
        running_ = true;
        watcher_thread_ = std::thread(&FileSystemWatcher::watchLoop, this);
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
    struct WatchInfo {
        Callback callback;
        std::unordered_map<std::string, fs::file_time_type> file_states;
    };

    std::unordered_map<fs::path, WatchInfo> watch_paths_;
    std::atomic<bool> running_;
    std::thread watcher_thread_;
    std::mutex mutex_;

    std::unordered_map<std::string, fs::file_time_type> getCurrentFileState(const fs::path& directory) {
        std::unordered_map<std::string, fs::file_time_type> states;
        try {
            for (const auto& entry : fs::recursive_directory_iterator(directory)) {
                if (fs::is_regular_file(entry.status())) {
                    states[entry.path().string()] = fs::last_write_time(entry);
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
        return states;
    }

    void watchLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto& [path, info] : watch_paths_) {
                auto current_states = getCurrentFileState(path);
                
                for (const auto& [file_path, current_time] : current_states) {
                    auto it = info.file_states.find(file_path);
                    if (it == info.file_states.end()) {
                        info.callback(file_path, "created");
                    } else if (it->second != current_time) {
                        info.callback(file_path, "modified");
                    }
                }

                for (const auto& [file_path, old_time] : info.file_states) {
                    if (current_states.find(file_path) == current_states.end()) {
                        info.callback(file_path, "deleted");
                    }
                }

                info.file_states = std::move(current_states);
            }
        }
    }
};

void exampleCallback(const fs::path& file_path, const std::string& action) {
    std::cout << "File: " << file_path << " Action: " << action << std::endl;
}

int main() {
    FileSystemWatcher watcher;
    
    watcher.addWatchPath("./test_directory", exampleCallback);
    
    std::cout << "Starting file system watcher. Monitoring './test_directory'" << std::endl;
    std::cout << "Press Enter to stop..." << std::endl;
    
    watcher.start();
    
    std::cin.get();
    
    watcher.stop();
    
    return 0;
}#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path path_to_watch;
    std::unordered_set<std::string> current_files;

    void rebuild_file_set() {
        current_files.clear();
        for (const auto& entry : fs::directory_iterator(path_to_watch)) {
            current_files.insert(entry.path().filename().string());
        }
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        rebuild_file_set();
        std::cout << "Watching directory: " << fs::absolute(path_to_watch) << std::endl;
    }

    void check_for_changes() {
        auto start_time = std::chrono::steady_clock::now();
        std::unordered_set<std::string> new_files;

        for (const auto& entry : fs::directory_iterator(path_to_watch)) {
            new_files.insert(entry.path().filename().string());
        }

        for (const auto& file : new_files) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "[+] File added: " << file << std::endl;
            }
        }

        for (const auto& file : current_files) {
            if (new_files.find(file) == new_files.end()) {
                std::cout << "[-] File removed: " << file << std::endl;
            }
        }

        current_files = std::move(new_files);

        auto end_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        std::cout << "Scan completed in " << elapsed.count() << " ms. Next scan in 2 seconds." << std::endl;
    }

    void start_monitoring(int interval_seconds = 2) {
        std::cout << "Starting monitoring. Press Ctrl+C to stop." << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            try {
                check_for_changes();
            } catch (const std::exception& e) {
                std::cerr << "Error during scan: " << e.what() << std::endl;
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
        std::cerr << "Failed to initialize watcher: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}