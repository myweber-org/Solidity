#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileWatcher {
public:
    FileWatcher(const fs::path& path_to_watch) : path_to_watch_(path_to_watch) {
        for (const auto& entry : fs::recursive_directory_iterator(path_to_watch_)) {
            if (fs::is_regular_file(entry.status())) {
                paths_[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void startMonitoring(int interval_seconds = 1) {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            checkForChanges();
        }
    }

    void stopMonitoring() {
        running_ = false;
    }

private:
    fs::path path_to_watch_;
    std::unordered_map<std::string, fs::file_time_type> paths_;
    bool running_ = true;

    void checkForChanges() {
        auto it = paths_.begin();
        while (it != paths_.end()) {
            if (!fs::exists(it->first)) {
                std::cout << "File deleted: " << it->first << std::endl;
                it = paths_.erase(it);
            } else {
                ++it;
            }
        }

        for (const auto& entry : fs::recursive_directory_iterator(path_to_watch_)) {
            if (fs::is_regular_file(entry.status())) {
                auto current_file_last_write_time = fs::last_write_time(entry);
                std::string file_path = entry.path().string();

                if (!paths_.contains(file_path)) {
                    paths_[file_path] = current_file_last_write_time;
                    std::cout << "File created: " << file_path << std::endl;
                } else {
                    auto& saved_last_write_time = paths_[file_path];
                    if (saved_last_write_time != current_file_last_write_time) {
                        saved_last_write_time = current_file_last_write_time;
                        std::cout << "File modified: " << file_path << std::endl;
                    }
                }
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    fs::path dir_to_watch = argv[1];
    if (!fs::exists(dir_to_watch) || !fs::is_directory(dir_to_watch)) {
        std::cerr << "Error: " << dir_to_watch << " is not a valid directory." << std::endl;
        return 1;
    }

    std::cout << "Starting file watcher on: " << fs::absolute(dir_to_watch) << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;

    FileWatcher watcher(dir_to_watch);
    watcher.startMonitoring(2);

    return 0;
}