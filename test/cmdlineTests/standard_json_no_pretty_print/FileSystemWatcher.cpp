#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <functional>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const fs::file_time_type&)>;

    explicit FileSystemWatcher(const fs::path& directory) : watch_directory(directory) {
        if (!fs::exists(watch_directory) || !fs::is_directory(watch_directory)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        initializeSnapshot();
    }

    void setUpdateCallback(Callback cb) {
        update_callback = std::move(cb);
    }

    void setDeleteCallback(Callback cb) {
        delete_callback = std::move(cb);
    }

    void startWatching(std::chrono::milliseconds interval = std::chrono::milliseconds(1000)) {
        running = true;
        while (running) {
            std::this_thread::sleep_for(interval);
            checkForChanges();
        }
    }

    void stopWatching() {
        running = false;
    }

private:
    fs::path watch_directory;
    std::unordered_map<std::string, fs::file_time_type> file_snapshot;
    Callback update_callback;
    Callback delete_callback;
    bool running = false;

    void initializeSnapshot() {
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.status())) {
                file_snapshot[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void checkForChanges() {
        std::unordered_map<std::string, fs::file_time_type> current_state;

        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.status())) {
                std::string path_str = entry.path().string();
                auto current_time = fs::last_write_time(entry);
                current_state[path_str] = current_time;

                auto it = file_snapshot.find(path_str);
                if (it == file_snapshot.end()) {
                    if (update_callback) {
                        update_callback(entry.path(), current_time);
                    }
                } else if (it->second != current_time) {
                    if (update_callback) {
                        update_callback(entry.path(), current_time);
                    }
                }
            }
        }

        for (const auto& [path, time] : file_snapshot) {
            if (current_state.find(path) == current_state.end()) {
                if (delete_callback) {
                    delete_callback(fs::path(path), time);
                }
            }
        }

        file_snapshot.swap(current_state);
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        
        watcher.setUpdateCallback([](const fs::path& path, const fs::file_time_type& time) {
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                time - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
            std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
            std::cout << "File updated: " << path << " at " << std::ctime(&cftime);
        });

        watcher.setDeleteCallback([](const fs::path& path, const fs::file_time_type& time) {
            std::cout << "File deleted: " << path << std::endl;
        });

        std::cout << "Starting file system watcher. Press Ctrl+C to stop." << std::endl;
        watcher.startWatching();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}