#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& directory) : watch_directory(directory) {
        if (!fs::exists(watch_directory) || !fs::is_directory(watch_directory)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        populate_snapshot();
    }

    void start_monitoring(int interval_seconds = 2) {
        std::cout << "Starting to monitor directory: " << watch_directory << "\n";
        while (monitoring_active) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

    void stop_monitoring() {
        monitoring_active = false;
    }

private:
    fs::path watch_directory;
    std::unordered_map<std::string, fs::file_time_type> file_snapshot;
    bool monitoring_active{true};

    void populate_snapshot() {
        file_snapshot.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                file_snapshot[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void check_for_changes() {
        auto current_state = file_snapshot;
        bool changes_detected = false;

        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                std::string file_path = entry.path().string();
                auto current_write_time = fs::last_write_time(entry);

                if (file_snapshot.find(file_path) == file_snapshot.end()) {
                    std::cout << "[NEW] File created: " << file_path << "\n";
                    changes_detected = true;
                } else if (file_snapshot[file_path] != current_write_time) {
                    std::cout << "[MODIFIED] File changed: " << file_path << "\n";
                    changes_detected = true;
                }
                current_state[file_path] = current_write_time;
            }
        }

        for (const auto& [file_path, _] : file_snapshot) {
            if (current_state.find(file_path) == current_state.end()) {
                std::cout << "[DELETED] File removed: " << file_path << "\n";
                changes_detected = true;
            }
        }

        if (changes_detected) {
            file_snapshot.swap(current_state);
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start_monitoring(3);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}