#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileWatcher {
public:
    FileWatcher(const std::string& path_to_watch, std::chrono::duration<int, std::milli> delay)
        : path_to_watch_{path_to_watch}, delay_{delay} {
        if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
            throw std::invalid_argument("Path does not exist or is not a directory");
        }
        populateFileMap();
    }

    void startMonitoring() {
        while (running_) {
            std::this_thread::sleep_for(delay_);
            checkForChanges();
        }
    }

    void stopMonitoring() {
        running_ = false;
    }

private:
    std::string path_to_watch_;
    std::chrono::duration<int, std::milli> delay_;
    std::unordered_map<std::string, fs::file_time_type> file_map_;
    bool running_{true};

    void populateFileMap() {
        file_map_.clear();
        for (const auto& entry : fs::recursive_directory_iterator(path_to_watch_)) {
            if (fs::is_regular_file(entry.path())) {
                file_map_[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void checkForChanges() {
        auto it = file_map_.begin();
        while (it != file_map_.end()) {
            if (!fs::exists(it->first)) {
                std::cout << "File deleted: " << it->first << std::endl;
                it = file_map_.erase(it);
            } else {
                ++it;
            }
        }

        for (const auto& entry : fs::recursive_directory_iterator(path_to_watch_)) {
            if (fs::is_regular_file(entry.path())) {
                std::string file_path = entry.path().string();
                auto current_write_time = fs::last_write_time(entry);

                if (file_map_.find(file_path) == file_map_.end()) {
                    std::cout << "File created: " << file_path << std::endl;
                    file_map_[file_path] = current_write_time;
                } else {
                    auto& stored_write_time = file_map_[file_path];
                    if (stored_write_time != current_write_time) {
                        std::cout << "File modified: " << file_path << std::endl;
                        stored_write_time = current_write_time;
                    }
                }
            }
        }
    }
};

int main() {
    try {
        FileWatcher fw{"./test_dir", std::chrono::milliseconds(2000)};
        std::cout << "Starting file system monitor. Press Ctrl+C to stop." << std::endl;
        fw.startMonitoring();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}