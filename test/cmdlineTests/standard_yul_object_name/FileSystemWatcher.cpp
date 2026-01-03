
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <iostream>
#include <atomic>
#include <functional>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;
    
    enum class EventType {
        CREATED,
        MODIFIED,
        DELETED,
        RENAMED
    };
    
    FileSystemWatcher() : running_(false) {}
    
    ~FileSystemWatcher() {
        stop();
    }
    
    void addWatch(const fs::path& directory, bool recursive, Callback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        watches_.push_back({directory, recursive, callback, {}});
        scanDirectory(watches_.back());
    }
    
    void start() {
        if (running_) return;
        
        running_ = true;
        worker_ = std::thread([this]() {
            while (running_) {
                std::this_thread::sleep_for(scan_interval_);
                checkForChanges();
            }
        });
    }
    
    void stop() {
        running_ = false;
        if (worker_.joinable()) {
            worker_.join();
        }
    }
    
    void setScanInterval(std::chrono::milliseconds interval) {
        scan_interval_ = interval;
    }
    
private:
    struct WatchInfo {
        fs::path directory;
        bool recursive;
        Callback callback;
        std::unordered_map<std::string, fs::file_time_type> file_timestamps;
    };
    
    struct FileInfo {
        fs::file_time_type last_write;
        uintmax_t size;
    };
    
    void scanDirectory(WatchInfo& watch) {
        try {
            auto scan = [&](const fs::path& path, auto&& scan_ref) -> void {
                for (const auto& entry : fs::directory_iterator(path)) {
                    const auto& filepath = entry.path();
                    std::string key = filepath.string();
                    
                    if (fs::is_regular_file(entry.status())) {
                        watch.file_timestamps[key] = fs::last_write_time(filepath);
                    }
                    
                    if (watch.recursive && fs::is_directory(entry.status())) {
                        scan_ref(filepath, scan_ref);
                    }
                }
            };
            
            scan(watch.directory, scan);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
    }
    
    void checkForChanges() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (auto& watch : watches_) {
            std::unordered_map<std::string, fs::file_time_type> current_files;
            std::unordered_map<std::string, FileInfo> current_info;
            
            try {
                auto scan = [&](const fs::path& path, auto&& scan_ref) -> void {
                    for (const auto& entry : fs::directory_iterator(path)) {
                        const auto& filepath = entry.path();
                        std::string key = filepath.string();
                        
                        if (fs::is_regular_file(entry.status())) {
                            auto last_write = fs::last_write_time(filepath);
                            current_files[key] = last_write;
                            current_info[key] = {last_write, fs::file_size(filepath)};
                        }
                        
                        if (watch.recursive && fs::is_directory(entry.status())) {
                            scan_ref(filepath, scan_ref);
                        }
                    }
                };
                
                scan(watch.directory, scan);
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Filesystem error: " << e.what() << std::endl;
                continue;
            }
            
            for (const auto& [filepath, current_time] : current_files) {
                auto it = watch.file_timestamps.find(filepath);
                if (it == watch.file_timestamps.end()) {
                    watch.callback(fs::path(filepath), "CREATED");
                } else if (it->second != current_time) {
                    watch.callback(fs::path(filepath), "MODIFIED");
                }
                watch.file_timestamps[filepath] = current_time;
            }
            
            for (auto it = watch.file_timestamps.begin(); it != watch.file_timestamps.end();) {
                if (current_files.find(it->first) == current_files.end()) {
                    watch.callback(fs::path(it->first), "DELETED");
                    it = watch.file_timestamps.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
    
    std::vector<WatchInfo> watches_;
    std::thread worker_;
    std::mutex mutex_;
    std::atomic<bool> running_;
    std::chrono::milliseconds scan_interval_{500};
};

void exampleCallback(const fs::path& path, const std::string& event) {
    std::cout << "File: " << path.filename() << " Event: " << event << std::endl;
}

int main() {
    FileSystemWatcher watcher;
    
    watcher.addWatch("./test_directory", true, exampleCallback);
    watcher.setScanInterval(std::chrono::milliseconds(1000));
    
    std::cout << "Starting file system watcher. Monitoring ./test_directory" << std::endl;
    std::cout << "Press Enter to stop..." << std::endl;
    
    watcher.start();
    
    std::cin.get();
    
    watcher.stop();
    
    return 0;
}