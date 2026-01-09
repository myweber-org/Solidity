
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
    using Callback = std::function<void(const fs::path&, fs::file_time_type)>;
    
    explicit FileSystemWatcher(const fs::path& directory) 
        : watch_directory(directory), running(false) {
        if (!fs::exists(watch_directory) || !fs::is_directory(watch_directory)) {
            throw std::runtime_error("Invalid directory path provided");
        }
    }
    
    void start(Callback callback) {
        if (running) return;
        
        running = true;
        snapshot_current_state();
        
        watcher_thread = std::thread([this, callback]() {
            while (running) {
                std::this_thread::sleep_for(poll_interval);
                
                std::lock_guard<std::mutex> lock(state_mutex);
                auto current_state = capture_current_state();
                
                for (const auto& [path, time] : current_state) {
                    auto it = file_snapshots.find(path);
                    if (it == file_snapshots.end()) {
                        file_snapshots[path] = time;
                        callback(path, time);
                    } else if (it->second != time) {
                        it->second = time;
                        callback(path, time);
                    }
                }
                
                for (auto it = file_snapshots.begin(); it != file_snapshots.end();) {
                    if (current_state.find(it->first) == current_state.end()) {
                        callback(it->first, fs::file_time_type::min());
                        it = file_snapshots.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
        });
    }
    
    void stop() {
        running = false;
        if (watcher_thread.joinable()) {
            watcher_thread.join();
        }
    }
    
    ~FileSystemWatcher() {
        stop();
    }
    
    void set_poll_interval(std::chrono::milliseconds interval) {
        poll_interval = interval;
    }
    
private:
    void snapshot_current_state() {
        file_snapshots.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.status())) {
                file_snapshots[entry.path()] = fs::last_write_time(entry);
            }
        }
    }
    
    std::unordered_map<fs::path, fs::file_time_type> capture_current_state() const {
        std::unordered_map<fs::path, fs::file_time_type> current;
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.status())) {
                current[entry.path()] = fs::last_write_time(entry);
            }
        }
        return current;
    }
    
    fs::path watch_directory;
    std::unordered_map<fs::path, fs::file_time_type> file_snapshots;
    std::chrono::milliseconds poll_interval{1000};
    std::atomic<bool> running;
    std::thread watcher_thread;
    mutable std::mutex state_mutex;
};

void example_usage() {
    try {
        FileSystemWatcher watcher("./monitored_folder");
        
        watcher.set_poll_interval(std::chrono::milliseconds(500));
        
        auto callback = [](const fs::path& path, fs::file_time_type time) {
            if (time == fs::file_time_type::min()) {
                std::cout << "File deleted: " << path.filename() << std::endl;
            } else {
                std::cout << "File modified: " << path.filename() 
                          << " at " << std::chrono::system_clock::to_time_t(
                              std::chrono::file_clock::to_sys(time)) << std::endl;
            }
        };
        
        watcher.start(callback);
        
        std::cout << "Watching directory. Press Enter to stop..." << std::endl;
        std::cin.get();
        
        watcher.stop();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}