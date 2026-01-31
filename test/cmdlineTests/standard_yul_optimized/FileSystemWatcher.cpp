
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
        : watch_directory(watch_path), change_callback(callback), running(false) {
        if (!fs::exists(watch_directory) || !fs::is_directory(watch_directory)) {
            throw std::runtime_error("Invalid watch directory path");
        }
        initialize_file_states();
    }

    ~SimpleFileWatcher() {
        stop();
    }

    void start() {
        running = true;
        watcher_thread = std::thread(&SimpleFileWatcher::watch_loop, this);
    }

    void stop() {
        running = false;
        if (watcher_thread.joinable()) {
            watcher_thread.join();
        }
    }

private:
    struct FileState {
        std::time_t last_write_time;
        std::uintmax_t file_size;
    };

    void initialize_file_states() {
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                fs::path file_path = entry.path();
                FileState state;
                state.last_write_time = fs::last_write_time(file_path).time_since_epoch().count();
                state.file_size = entry.file_size();
                file_states[file_path] = state;
            }
        }
    }

    void watch_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            check_for_changes();
        }
    }

    void check_for_changes() {
        std::unordered_map<fs::path, FileState> current_states;

        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                fs::path file_path = entry.path();
                FileState state;
                state.last_write_time = fs::last_write_time(file_path).time_since_epoch().count();
                state.file_size = entry.file_size();
                current_states[file_path] = state;

                auto it = file_states.find(file_path);
                if (it == file_states.end()) {
                    change_callback(file_path, "CREATED");
                } else {
                    const FileState& old_state = it->second;
                    if (state.last_write_time != old_state.last_write_time) {
                        change_callback(file_path, "MODIFIED");
                    }
                    file_states.erase(it);
                }
            }
        }

        for (const auto& [deleted_file, _] : file_states) {
            change_callback(deleted_file, "DELETED");
        }

        file_states = std::move(current_states);
    }

    fs::path watch_directory;
    FileChangeCallback change_callback;
    std::unordered_map<fs::path, FileState> file_states;
    std::thread watcher_thread;
    std::atomic<bool> running;
};

void example_callback(const fs::path& file_path, const std::string& change_type) {
    std::cout << "File: " << file_path << " - Change: " << change_type << std::endl;
}

int main() {
    try {
        fs::path current_dir = fs::current_path();
        SimpleFileWatcher watcher(current_dir, example_callback);
        
        std::cout << "Watching directory: " << current_dir << std::endl;
        std::cout << "Press Enter to stop watching..." << std::endl;
        
        watcher.start();
        std::cin.get();
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
    bool running_;
};

void exampleCallback(const fs::path& file_path, const std::string& change_type) {
    std::cout << "File: " << file_path.string() << " - Action: " << change_type << std::endl;
}

int main() {
    try {
        fs::path watch_directory = ".";
        SimpleFileWatcher watcher(watch_directory, exampleCallback);
        
        std::cout << "Watching directory: " << fs::absolute(watch_directory).string() << std::endl;
        std::cout << "Press Enter to stop watching..." << std::endl;
        
        watcher.start();
        std::cin.get();
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
    explicit FileSystemWatcher(const fs::path& path) : watch_path_(path) {
        if (!fs::exists(watch_path_) || !fs::is_directory(watch_path_)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        cache_contents();
    }

    void start_monitoring(int interval_seconds = 1) {
        std::cout << "Monitoring directory: " << watch_path_ << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path watch_path_;
    std::unordered_set<std::string> file_cache_;

    void cache_contents() {
        file_cache_.clear();
        for (const auto& entry : fs::directory_iterator(watch_path_)) {
            file_cache_.insert(entry.path().filename().string());
        }
    }

    void check_for_changes() {
        auto current_files = std::unordered_set<std::string>{};
        for (const auto& entry : fs::directory_iterator(watch_path_)) {
            current_files.insert(entry.path().filename().string());
        }

        for (const auto& file : current_files) {
            if (file_cache_.find(file) == file_cache_.end()) {
                std::cout << "File added: " << file << std::endl;
            }
        }

        for (const auto& file : file_cache_) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "File removed: " << file << std::endl;
            }
        }

        file_cache_ = std::move(current_files);
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
}