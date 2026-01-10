
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

    FileSystemWatcher(const fs::path& directory, Callback callback)
        : watch_directory(directory), change_callback(callback), running(false) {
        
        if (!fs::exists(watch_directory) || !fs::is_directory(watch_directory)) {
            throw std::runtime_error("Invalid directory path provided");
        }
    }

    void start() {
        running = true;
        scan_initial_state();
        watcher_thread = std::thread(&FileSystemWatcher::watch_loop, this);
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

private:
    void scan_initial_state() {
        file_states.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.path())) {
                file_states[entry.path()] = get_file_signature(entry.path());
            }
        }
    }

    std::string get_file_signature(const fs::path& file_path) {
        auto ftime = fs::last_write_time(file_path);
        auto size = fs::file_size(file_path);
        return std::to_string(size) + "_" + std::to_string(ftime.time_since_epoch().count());
    }

    void watch_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            detect_changes();
        }
    }

    void detect_changes() {
        std::unordered_map<fs::path, std::string> current_states;

        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.path())) {
                current_states[entry.path()] = get_file_signature(entry.path());
            }
        }

        for (const auto& [path, signature] : current_states) {
            auto it = file_states.find(path);
            if (it == file_states.end()) {
                change_callback(path, "created");
            } else if (it->second != signature) {
                change_callback(path, "modified");
            }
        }

        for (const auto& [path, signature] : file_states) {
            if (current_states.find(path) == current_states.end()) {
                change_callback(path, "deleted");
            }
        }

        file_states = std::move(current_states);
    }

    fs::path watch_directory;
    Callback change_callback;
    std::unordered_map<fs::path, std::string> file_states;
    std::thread watcher_thread;
    bool running;
};

void example_callback(const fs::path& path, const std::string& action) {
    std::cout << "File: " << path << " Action: " << action << std::endl;
}

int main() {
    try {
        FileSystemWatcher watcher(".", example_callback);
        watcher.start();
        
        std::cout << "Watching current directory. Press Enter to stop..." << std::endl;
        std::cin.get();
        
        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}