
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>
#include <functional>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher(const fs::path& directory, Callback callback)
        : watch_directory(directory), event_callback(callback), running(false) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            throw std::runtime_error("Invalid directory path");
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

    void start() {
        running = true;
        scan_initial_state();
        monitor_thread = std::thread(&FileSystemWatcher::monitor_loop, this);
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

    void scan_initial_state() {
        file_states.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                file_states[entry.path()] = get_file_state(entry.path());
            }
        }
    }

    FileState get_file_state(const fs::path& file_path) {
        FileState state{};
#ifdef _WIN32
        WIN32_FILE_ATTRIBUTE_DATA file_attr;
        if (GetFileAttributesExW(file_path.c_str(), GetFileExInfoStandard, &file_attr)) {
            state.last_write_time = (static_cast<std::time_t>(file_attr.ftLastWriteTime.dwHighDateTime) << 32) |
                                     file_attr.ftLastWriteTime.dwLowDateTime;
            state.file_size = (static_cast<std::uintmax_t>(file_attr.nFileSizeHigh) << 32) |
                               file_attr.nFileSizeLow;
        }
#else
        struct stat file_stat;
        if (stat(file_path.c_str(), &file_stat) == 0) {
            state.last_write_time = file_stat.st_mtime;
            state.file_size = file_stat.st_size;
        }
#endif
        return state;
    }

    void monitor_loop() {
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
                FileState current_state = get_file_state(file_path);
                current_states[file_path] = current_state;

                auto it = file_states.find(file_path);
                if (it == file_states.end()) {
                    event_callback(file_path, "CREATED");
                } else if (it->second.last_write_time != current_state.last_write_time ||
                           it->second.file_size != current_state.file_size) {
                    event_callback(file_path, "MODIFIED");
                }
            }
        }

        for (const auto& [old_path, _] : file_states) {
            if (current_states.find(old_path) == current_states.end()) {
                event_callback(old_path, "DELETED");
            }
        }

        file_states.swap(current_states);
    }

    fs::path watch_directory;
    Callback event_callback;
    std::thread monitor_thread;
    std::unordered_map<fs::path, FileState> file_states;
    std::atomic<bool> running;
};

void example_usage() {
    FileSystemWatcher watcher(fs::current_path(), [](const fs::path& path, const std::string& event) {
        std::cout << "File: " << path.filename() << " Event: " << event << std::endl;
    });

    watcher.start();
    std::this_thread::sleep_for(std::chrono::seconds(30));
    watcher.stop();
}