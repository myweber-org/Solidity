
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    SimpleFileWatcher(const std::string& path) : watch_path(path), running(false) {
        if (!fs::exists(path) || !fs::is_directory(path)) {
            throw std::runtime_error("Path does not exist or is not a directory");
        }
        build_file_map();
    }

    void start(int interval_seconds = 1) {
        running = true;
        std::cout << "Watching directory: " << watch_path << std::endl;
        
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

    void stop() {
        running = false;
    }

private:
    std::string watch_path;
    bool running;
    std::unordered_map<std::string, fs::file_time_type> file_map;

    void build_file_map() {
        file_map.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                file_map[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void check_for_changes() {
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                std::string file_path = entry.path().string();
                auto current_time = fs::last_write_time(entry);

                if (file_map.find(file_path) == file_map.end()) {
                    std::cout << "[NEW] " << file_path << std::endl;
                    file_map[file_path] = current_time;
                } else if (file_map[file_path] != current_time) {
                    std::cout << "[MODIFIED] " << file_path << std::endl;
                    file_map[file_path] = current_time;
                }
            }
        }

        std::vector<std::string> to_remove;
        for (const auto& [file_path, _] : file_map) {
            if (!fs::exists(file_path)) {
                std::cout << "[DELETED] " << file_path << std::endl;
                to_remove.push_back(file_path);
            }
        }

        for (const auto& file_path : to_remove) {
            file_map.erase(file_path);
        }
    }
};

int main() {
    try {
        SimpleFileWatcher watcher(".");
        watcher.start(2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}