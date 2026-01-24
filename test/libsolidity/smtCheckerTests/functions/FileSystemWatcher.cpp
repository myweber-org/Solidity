
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    using FileTimeMap = std::unordered_map<std::string, fs::file_time_type>;

    SimpleFileWatcher(const std::string& path) : watch_path_(path) {
        if (!fs::exists(path) || !fs::is_directory(path)) {
            throw std::runtime_error("Path does not exist or is not a directory");
        }
        populate_initial_state();
    }

    void start_watching(int interval_seconds = 2) {
        std::cout << "Watching directory: " << watch_path_ << std::endl;
        std::cout << "Polling interval: " << interval_seconds << " seconds" << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    void populate_initial_state() {
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (entry.is_regular_file()) {
                std::string file_path = entry.path().string();
                file_states_[file_path] = fs::last_write_time(entry);
            }
        }
    }

    void check_for_changes() {
        FileTimeMap current_states;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (entry.is_regular_file()) {
                std::string file_path = entry.path().string();
                current_states[file_path] = fs::last_write_time(entry);

                auto old_state = file_states_.find(file_path);
                if (old_state == file_states_.end()) {
                    std::cout << "[NEW] " << file_path << std::endl;
                } else if (old_state->second != current_states[file_path]) {
                    std::cout << "[MODIFIED] " << file_path << std::endl;
                }
            }
        }

        for (const auto& old_file : file_states_) {
            if (current_states.find(old_file.first) == current_states.end()) {
                std::cout << "[DELETED] " << old_file.first << std::endl;
            }
        }

        file_states_.swap(current_states);
    }

    std::string watch_path_;
    FileTimeMap file_states_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        SimpleFileWatcher watcher(argv[1]);
        watcher.start_watching();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}