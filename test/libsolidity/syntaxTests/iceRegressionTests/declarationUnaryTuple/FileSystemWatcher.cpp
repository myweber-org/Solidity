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
    using FileChangeCallback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher() : running_(false) {}

    void addWatchPath(const fs::path& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (fs::exists(path) && fs::is_directory(path)) {
            watch_paths_.push_back(path);
            scanPath(path);
        }
    }

    void setCallback(FileChangeCallback callback) {
        callback_ = std::move(callback);
    }

    void start() {
        running_ = true;
        monitor_thread_ = std::thread(&FileSystemWatcher::monitorLoop, this);
    }

    void stop() {
        running_ = false;
        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

private:
    struct FileMetadata {
        std::uintmax_t size;
        std::time_t last_write_time;
    };

    void scanPath(const fs::path& path) {
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (entry.is_regular_file()) {
                FileMetadata meta;
                meta.size = entry.file_size();
                meta.last_write_time = fs::last_write_time(entry).time_since_epoch().count();
                file_cache_[entry.path()] = meta;
            }
        }
    }

    void monitorLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            checkForChanges();
        }
    }

    void checkForChanges() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& watch_path : watch_paths_) {
            if (!fs::exists(watch_path)) continue;

            for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
                if (entry.is_regular_file()) {
                    const auto& current_path = entry.path();
                    FileMetadata current_meta;
                    current_meta.size = entry.file_size();
                    current_meta.last_write_time = fs::last_write_time(entry).time_since_epoch().count();

                    auto it = file_cache_.find(current_path);
                    if (it == file_cache_.end()) {
                        file_cache_[current_path] = current_meta;
                        if (callback_) {
                            callback_(current_path, "CREATED");
                        }
                    } else {
                        const auto& cached_meta = it->second;
                        if (cached_meta.size != current_meta.size ||
                            cached_meta.last_write_time != current_meta.last_write_time) {
                            file_cache_[current_path] = current_meta;
                            if (callback_) {
                                callback_(current_path, "MODIFIED");
                            }
                        }
                    }
                }
            }

            checkForDeletions();
        }
    }

    void checkForDeletions() {
        auto it = file_cache_.begin();
        while (it != file_cache_.end()) {
            if (!fs::exists(it->first)) {
                if (callback_) {
                    callback_(it->first, "DELETED");
                }
                it = file_cache_.erase(it);
            } else {
                ++it;
            }
        }
    }

    std::vector<fs::path> watch_paths_;
    std::unordered_map<fs::path, FileMetadata> file_cache_;
    FileChangeCallback callback_;
    std::atomic<bool> running_;
    std::thread monitor_thread_;
    std::mutex mutex_;
};

int main() {
    FileSystemWatcher watcher;
    
    watcher.setCallback([](const fs::path& path, const std::string& action) {
        std::cout << "File: " << path.string() << " Action: " << action << std::endl;
    });

    watcher.addWatchPath(".");
    
    std::cout << "Starting file system watcher. Monitoring current directory." << std::endl;
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
public:
    explicit FileSystemWatcher(const fs::path& path) : watch_path_(path) {
        if (!fs::exists(watch_path_) || !fs::is_directory(watch_path_)) {
            throw std::runtime_error("Provided path is not a valid directory.");
        }
        cache_contents();
    }

    void start_watching(int interval_seconds = 2) {
        std::cout << "Watching directory: " << watch_path_ << std::endl;
        std::cout << "Press Ctrl+C to stop." << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path watch_path_;
    std::unordered_set<std::string> cached_files_;

    void cache_contents() {
        cached_files_.clear();
        for (const auto& entry : fs::directory_iterator(watch_path_)) {
            cached_files_.insert(entry.path().filename().string());
        }
    }

    void check_for_changes() {
        auto current_files = std::unordered_set<std::string>{};
        for (const auto& entry : fs::directory_iterator(watch_path_)) {
            current_files.insert(entry.path().filename().string());
        }

        for (const auto& file : current_files) {
            if (cached_files_.find(file) == cached_files_.end()) {
                std::cout << "[+] File added: " << file << std::endl;
            }
        }

        for (const auto& file : cached_files_) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "[-] File removed: " << file << std::endl;
            }
        }

        cached_files_ = std::move(current_files);
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