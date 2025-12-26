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
        if (fs::exists(path) && fs::is_directory(path)) {
            refresh_file_map();
        }
    }

    void start(int interval_seconds = 2) {
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

    void refresh_file_map() {
        file_map.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_map[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void check_for_changes() {
        auto current_files = file_map;
        refresh_file_map();

        for (const auto& [path, mtime] : file_map) {
            auto old_it = current_files.find(path);
            if (old_it == current_files.end()) {
                std::cout << "[NEW] " << path << std::endl;
            } else if (old_it->second != mtime) {
                std::cout << "[MODIFIED] " << path << std::endl;
            }
        }

        for (const auto& [path, mtime] : current_files) {
            if (file_map.find(path) == file_map.end()) {
                std::cout << "[DELETED] " << path << std::endl;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    SimpleFileWatcher watcher(argv[1]);
    
    std::thread watch_thread([&watcher]() {
        watcher.start();
    });

    std::cout << "Press Enter to stop watching..." << std::endl;
    std::cin.get();
    
    watcher.stop();
    watch_thread.join();
    
    return 0;
}