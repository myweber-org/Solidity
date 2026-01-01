
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
            throw std::runtime_error("Invalid watch directory");
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
        file_states.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                fs::path file_path = entry.path();
                auto ftime = fs::last_write_time(file_path);
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
                std::time_t last_write = std::chrono::system_clock::to_time_t(sctp);
                std::uintmax_t size = entry.file_size();
                file_states[file_path] = {last_write, size};
            }
        }
    }

    void watch_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            check_for_changes();
        }
    }

    void check_for_changes() {
        std::unordered_map<fs::path, FileState> current_states;

        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                fs::path file_path = entry.path();
                auto ftime = fs::last_write_time(file_path);
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
                std::time_t last_write = std::chrono::system_clock::to_time_t(sctp);
                std::uintmax_t size = entry.file_size();
                current_states[file_path] = {last_write, size};

                auto it = file_states.find(file_path);
                if (it == file_states.end()) {
                    change_callback(file_path, "CREATED");
                } else if (it->second.last_write_time != last_write || it->second.file_size != size) {
                    change_callback(file_path, "MODIFIED");
                }
            }
        }

        for (const auto& [old_path, state] : file_states) {
            if (current_states.find(old_path) == current_states.end()) {
                change_callback(old_path, "DELETED");
            }
        }

        file_states.swap(current_states);
    }

    fs::path watch_directory;
    FileChangeCallback change_callback;
    std::unordered_map<fs::path, FileState> file_states;
    std::thread watcher_thread;
    std::atomic<bool> running;
};

void example_callback(const fs::path& file_path, const std::string& change_type) {
    std::cout << "File: " << file_path.string() << " - Change: " << change_type << std::endl;
}

int main() {
    try {
        fs::path current_path = fs::current_path();
        SimpleFileWatcher watcher(current_path, example_callback);
        
        std::cout << "Watching directory: " << current_path.string() << std::endl;
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