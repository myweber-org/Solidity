
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
        : watch_directory(watch_path), callback_func(callback), running(false) {
        if (!fs::exists(watch_directory) || !fs::is_directory(watch_directory)) {
            throw std::runtime_error("Invalid watch directory path");
        }
        initializeFileStates();
    }

    ~SimpleFileWatcher() {
        stop();
    }

    void start(int interval_ms = 1000) {
        running = true;
        monitor_thread = std::thread(&SimpleFileWatcher::monitorLoop, this, interval_ms);
    }

    void stop() {
        running = false;
        if (monitor_thread.joinable()) {
            monitor_thread.join();
        }
    }

private:
    struct FileState {
        std::time_t last_write_time;
        std::uintmax_t file_size;
    };

    void initializeFileStates() {
        file_states.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                auto path = entry.path();
                auto ftime = fs::last_write_time(path);
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
                std::time_t last_write = std::chrono::system_clock::to_time_t(sctp);
                file_states[path] = {last_write, entry.file_size()};
            }
        }
    }

    void monitorLoop(int interval_ms) {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
            checkForChanges();
        }
    }

    void checkForChanges() {
        std::unordered_map<fs::path, FileState> current_states;

        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                auto path = entry.path();
                auto ftime = fs::last_write_time(path);
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
                std::time_t last_write = std::chrono::system_clock::to_time_t(sctp);
                current_states[path] = {last_write, entry.file_size()};
            }
        }

        for (const auto& [path, current_state] : current_states) {
            auto it = file_states.find(path);
            if (it == file_states.end()) {
                callback_func(path, "CREATED");
            } else if (it->second.last_write_time != current_state.last_write_time ||
                       it->second.file_size != current_state.file_size) {
                callback_func(path, "MODIFIED");
            }
        }

        for (const auto& [path, _] : file_states) {
            if (current_states.find(path) == current_states.end()) {
                callback_func(path, "DELETED");
            }
        }

        file_states = std::move(current_states);
    }

    fs::path watch_directory;
    FileChangeCallback callback_func;
    std::unordered_map<fs::path, FileState> file_states;
    std::thread monitor_thread;
    std::atomic<bool> running;
};

void exampleCallback(const fs::path& file_path, const std::string& change_type) {
    std::cout << "File: " << file_path.string() << " - Change: " << change_type << std::endl;
}

int main() {
    try {
        fs::path current_path = fs::current_path();
        SimpleFileWatcher watcher(current_path, exampleCallback);
        
        std::cout << "Watching directory: " << current_path.string() << std::endl;
        std::cout << "Press Enter to stop watching..." << std::endl;
        
        watcher.start(2000);
        
        std::cin.get();
        
        watcher.stop();
        std::cout << "File watching stopped." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}