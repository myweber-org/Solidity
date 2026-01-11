
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
            updateSnapshot();
        }
    }

    void start() {
        running = true;
        std::cout << "Watching directory: " << watch_path << std::endl;
        
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            
            auto current_state = getCurrentState();
            
            for (const auto& [path, time] : current_state) {
                if (file_snapshot.find(path) == file_snapshot.end()) {
                    std::cout << "Created: " << path << std::endl;
                }
            }
            
            for (const auto& [path, time] : file_snapshot) {
                if (current_state.find(path) == current_state.end()) {
                    std::cout << "Deleted: " << path << std::endl;
                } else if (current_state[path] != time) {
                    std::cout << "Modified: " << path << std::endl;
                }
            }
            
            file_snapshot = current_state;
        }
    }

    void stop() {
        running = false;
    }

private:
    std::string watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_snapshot;
    bool running;

    void updateSnapshot() {
        file_snapshot.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_snapshot[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    std::unordered_map<std::string, fs::file_time_type> getCurrentState() {
        std::unordered_map<std::string, fs::file_time_type> current;
        try {
            for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
                if (fs::is_regular_file(entry.path())) {
                    current[entry.path().string()] = fs::last_write_time(entry);
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
        return current;
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