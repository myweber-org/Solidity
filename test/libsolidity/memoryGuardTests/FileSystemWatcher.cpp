#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& dir_path) : directory_path(dir_path) {
        if (!fs::exists(directory_path) || !fs::is_directory(directory_path)) {
            throw std::runtime_error("Provided path is not a valid directory.");
        }
        populate_snapshot();
    }

    void start_monitoring(int interval_seconds = 1) {
        std::cout << "Starting to monitor: " << directory_path << std::endl;
        while (monitoring_active) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

    void stop_monitoring() {
        monitoring_active = false;
    }

private:
    fs::path directory_path;
    std::unordered_map<std::string, fs::file_time_type> file_snapshot;
    bool monitoring_active{true};

    void populate_snapshot() {
        file_snapshot.clear();
        for (const auto& entry : fs::recursive_directory_iterator(directory_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_snapshot[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void check_for_changes() {
        std::unordered_map<std::string, fs::file_time_type> current_state;

        for (const auto& entry : fs::recursive_directory_iterator(directory_path)) {
            if (fs::is_regular_file(entry.path())) {
                std::string file_path = entry.path().string();
                auto current_write_time = fs::last_write_time(entry);
                current_state[file_path] = current_write_time;

                auto it = file_snapshot.find(file_path);
                if (it == file_snapshot.end()) {
                    std::cout << "[NEW] " << file_path << std::endl;
                } else if (it->second != current_write_time) {
                    std::cout << "[MODIFIED] " << file_path << std::endl;
                }
            }
        }

        for (const auto& [old_file, _] : file_snapshot) {
            if (current_state.find(old_file) == current_state.end()) {
                std::cout << "[DELETED] " << old_file << std::endl;
            }
        }

        file_snapshot.swap(current_state);
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start_monitoring(2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}