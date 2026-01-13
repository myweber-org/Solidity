
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
        : watch_directory(directory), notify_callback(callback), running(false) {
        if (!fs::exists(watch_directory) || !fs::is_directory(watch_directory)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        initialize_file_states();
    }

    ~FileSystemWatcher() {
        stop();
    }

    void start() {
        running = true;
        monitor_thread = std::thread(&FileSystemWatcher::monitor_loop, this);
    }

    void stop() {
        running = false;
        if (monitor_thread.joinable()) {
            monitor_thread.join();
        }
    }

private:
    fs::path watch_directory;
    Callback notify_callback;
    std::unordered_map<std::string, fs::file_time_type> file_states;
    std::thread monitor_thread;
    bool running;

    void initialize_file_states() {
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.status())) {
                file_states[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void monitor_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            auto current_states = scan_current_states();
            detect_changes(current_states);
        }
    }

    std::unordered_map<std::string, fs::file_time_type> scan_current_states() {
        std::unordered_map<std::string, fs::file_time_type> current;
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.status())) {
                current[entry.path().string()] = fs::last_write_time(entry);
            }
        }
        return current;
    }

    void detect_changes(const std::unordered_map<std::string, fs::file_time_type>& current) {
        for (const auto& [path, mtime] : current) {
            auto it = file_states.find(path);
            if (it == file_states.end()) {
                notify_callback(path, "created");
            } else if (it->second != mtime) {
                notify_callback(path, "modified");
            }
        }

        for (const auto& [path, _] : file_states) {
            if (current.find(path) == current.end()) {
                notify_callback(path, "deleted");
            }
        }

        file_states = current;
    }
};

void example_callback(const fs::path& file_path, const std::string& change_type) {
    std::cout << "File: " << file_path.filename() << " - Action: " << change_type << std::endl;
}

int main() {
    try {
        fs::path watch_path = fs::current_path() / "watch_folder";
        fs::create_directories(watch_path);

        FileSystemWatcher watcher(watch_path, example_callback);
        watcher.start();

        std::cout << "Watching directory: " << watch_path << std::endl;
        std::cout << "Press Enter to stop..." << std::endl;
        std::cin.get();

        watcher.stop();
        fs::remove_all(watch_path);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}