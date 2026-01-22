
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path watch_path;
    std::unordered_set<std::string> known_files;

    void scan_directory() {
        std::unordered_set<std::string> current_files;
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            current_files.insert(entry.path().filename().string());
        }

        for (const auto& file : current_files) {
            if (known_files.find(file) == known_files.end()) {
                std::cout << "[+] File added: " << file << std::endl;
            }
        }

        for (const auto& file : known_files) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "[-] File removed: " << file << std::endl;
            }
        }

        known_files = std::move(current_files);
    }

public:
    explicit FileSystemWatcher(const std::string& path) : watch_path(path) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Invalid directory path");
        }
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            known_files.insert(entry.path().filename().string());
        }
        std::cout << "Watching directory: " << watch_path << std::endl;
    }

    void start_watching(int interval_seconds = 2) {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            try {
                scan_directory();
            } catch (const std::exception& e) {
                std::cerr << "Error scanning directory: " << e.what() << std::endl;
            }
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
        watcher.start_watching();
    } catch (const std::exception& e) {
        std::cerr << "Failed to start watcher: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
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

    FileSystemWatcher(const fs::path& watch_path, Callback callback)
        : watch_directory_(watch_path), callback_(callback), running_(false) {
        
        if (!fs::exists(watch_directory_) || !fs::is_directory(watch_directory_)) {
            throw std::runtime_error("Invalid watch directory");
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

    void start() {
        if (running_) return;
        
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
    void monitor() {
        std::unordered_map<std::string, fs::file_time_type> file_timestamps;
        
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory_)) {
            if (fs::is_regular_file(entry.path())) {
                file_timestamps[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }

        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            std::unordered_map<std::string, fs::file_time_type> current_timestamps;
            
            for (const auto& entry : fs::recursive_directory_iterator(watch_directory_)) {
                if (!fs::is_regular_file(entry.path())) continue;
                
                std::string file_path = entry.path().string();
                auto current_time = fs::last_write_time(entry.path());
                current_timestamps[file_path] = current_time;

                auto it = file_timestamps.find(file_path);
                if (it == file_timestamps.end()) {
                    notify(entry.path(), "created");
                } else if (it->second != current_time) {
                    notify(entry.path(), "modified");
                }
            }

            for (const auto& [file_path, _] : file_timestamps) {
                if (current_timestamps.find(file_path) == current_timestamps.end()) {
                    notify(file_path, "deleted");
                }
            }

            file_timestamps = std::move(current_timestamps);
        }
    }

    void notify(const fs::path& file_path, const std::string& event_type) {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        if (callback_) {
            callback_(file_path, event_type);
        }
    }

    fs::path watch_directory_;
    Callback callback_;
    std::atomic<bool> running_;
    std::thread monitor_thread_;
    std::mutex callback_mutex_;
};

void example_callback(const fs::path& path, const std::string& event) {
    std::cout << "File: " << path.filename() << " Event: " << event 
              << " at " << std::chrono::system_clock::now().time_since_epoch().count() 
              << std::endl;
}

int main() {
    try {
        FileSystemWatcher watcher("./test_directory", example_callback);
        watcher.start();
        
        std::cout << "Watching directory. Press Enter to stop..." << std::endl;
        std::cin.get();
        
        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}