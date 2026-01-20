
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

    SimpleFileWatcher(const fs::path& directory, FileChangeCallback callback)
        : watch_directory(directory), change_callback(callback), running(false) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            throw std::runtime_error("Invalid directory path provided.");
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
    fs::path watch_directory;
    FileChangeCallback change_callback;
    std::unordered_map<std::string, fs::file_time_type> file_states;
    std::thread watcher_thread;
    bool running;

    void initialize_file_states() {
        file_states.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                file_states[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void watch_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            std::unordered_map<std::string, fs::file_time_type> current_states;

            for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
                if (entry.is_regular_file()) {
                    std::string file_path = entry.path().string();
                    auto current_time = fs::last_write_time(entry);
                    current_states[file_path] = current_time;

                    auto it = file_states.find(file_path);
                    if (it == file_states.end()) {
                        change_callback(entry.path(), "CREATED");
                    } else if (it->second != current_time) {
                        change_callback(entry.path(), "MODIFIED");
                    }
                }
            }

            for (const auto& old_file : file_states) {
                if (current_states.find(old_file.first) == current_states.end()) {
                    change_callback(fs::path(old_file.first), "DELETED");
                }
            }

            file_states.swap(current_states);
        }
    }
};

void example_callback(const fs::path& file_path, const std::string& change_type) {
    std::cout << "File: " << file_path << " - Change: " << change_type << std::endl;
}

int main() {
    try {
        fs::path watch_path = "./test_watch";
        fs::create_directories(watch_path);

        SimpleFileWatcher watcher(watch_path, example_callback);
        watcher.start();

        std::cout << "Watching directory: " << watch_path << std::endl;
        std::cout << "Press Enter to stop watching..." << std::endl;
        std::cin.get();

        watcher.stop();
        fs::remove_all(watch_path);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}