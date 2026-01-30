
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
    using FileTime = fs::file_time_type;
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher(const fs::path& directory, Callback callback)
        : watch_directory(directory), notify_callback(callback), running(false) {
        if (!fs::exists(watch_directory) || !fs::is_directory(watch_directory)) {
            throw std::runtime_error("Invalid directory path");
        }
    }

    void start() {
        running = true;
        scan_existing_files();
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
    fs::path watch_directory;
    Callback notify_callback;
    std::unordered_map<std::string, FileTime> file_timestamps;
    std::thread watcher_thread;
    bool running;

    void scan_existing_files() {
        for (const auto& entry : fs::directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.status())) {
                file_timestamps[entry.path().filename().string()] = fs::last_write_time(entry);
            }
        }
    }

    void watch_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            try {
                std::vector<std::string> current_files;
                for (const auto& entry : fs::directory_iterator(watch_directory)) {
                    if (fs::is_regular_file(entry.status())) {
                        std::string filename = entry.path().filename().string();
                        current_files.push_back(filename);

                        auto it = file_timestamps.find(filename);
                        if (it == file_timestamps.end()) {
                            file_timestamps[filename] = fs::last_write_time(entry);
                            notify_callback(entry.path(), "created");
                        } else {
                            FileTime current_time = fs::last_write_time(entry);
                            if (it->second != current_time) {
                                it->second = current_time;
                                notify_callback(entry.path(), "modified");
                            }
                        }
                    }
                }

                for (auto it = file_timestamps.begin(); it != file_timestamps.end();) {
                    if (std::find(current_files.begin(), current_files.end(), it->first) == current_files.end()) {
                        fs::path file_path = watch_directory / it->first;
                        notify_callback(file_path, "deleted");
                        it = file_timestamps.erase(it);
                    } else {
                        ++it;
                    }
                }
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Filesystem error: " << e.what() << std::endl;
            }
        }
    }
};

void example_callback(const fs::path& file_path, const std::string& change_type) {
    std::cout << "File: " << file_path.filename() << " - Action: " << change_type << std::endl;
}

int main() {
    try {
        fs::path current_dir = fs::current_path();
        FileSystemWatcher watcher(current_dir, example_callback);
        
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