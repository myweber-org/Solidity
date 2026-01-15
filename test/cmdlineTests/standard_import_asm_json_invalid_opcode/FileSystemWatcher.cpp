
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    SimpleFileWatcher(const std::string& path) : watch_path(path) {
        if (fs::exists(watch_path) && fs::is_directory(watch_path)) {
            refresh_file_states();
        }
    }

    void start_watching(int interval_seconds = 2) {
        std::cout << "Watching directory: " << watch_path << std::endl;
        std::cout << "Polling interval: " << interval_seconds << " seconds" << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    std::string watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_states;

    void refresh_file_states() {
        file_states.clear();
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_states[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void check_for_changes() {
        bool changes_detected = false;

        std::unordered_map<std::string, fs::file_time_type> current_states;

        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                std::string file_path = entry.path().string();
                auto current_time = fs::last_write_time(entry.path());
                current_states[file_path] = current_time;

                auto it = file_states.find(file_path);
                if (it == file_states.end()) {
                    std::cout << "[NEW] File created: " << file_path << std::endl;
                    changes_detected = true;
                } else if (it->second != current_time) {
                    std::cout << "[MODIFIED] File changed: " << file_path << std::endl;
                    changes_detected = true;
                }
            }
        }

        for (const auto& old_file : file_states) {
            if (current_states.find(old_file.first) == current_states.end()) {
                std::cout << "[DELETED] File removed: " << old_file.first << std::endl;
                changes_detected = true;
            }
        }

        if (changes_detected) {
            file_states = std::move(current_states);
        }
    }
};

int main(int argc, char* argv[]) {
    std::string path_to_watch = ".";
    if (argc > 1) {
        path_to_watch = argv[1];
    }

    try {
        SimpleFileWatcher watcher(path_to_watch);
        watcher.start_watching();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}