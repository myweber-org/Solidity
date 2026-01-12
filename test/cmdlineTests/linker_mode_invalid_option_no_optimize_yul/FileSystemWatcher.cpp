
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

    SimpleFileWatcher(const std::string& path) : watch_path(path), running(false) {
        if (fs::exists(path) && fs::is_directory(path)) {
            snapshot = get_current_snapshot();
        }
    }

    void start_watching(int interval_seconds = 2) {
        running = true;
        std::cout << "Watching directory: " << watch_path << std::endl;
        
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            
            auto current_snapshot = get_current_snapshot();
            detect_changes(current_snapshot);
            snapshot = std::move(current_snapshot);
        }
    }

    void stop_watching() {
        running = false;
    }

private:
    FileTimeMap get_current_snapshot() {
        FileTimeMap current;
        try {
            for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
                if (fs::is_regular_file(entry.path())) {
                    current[entry.path().string()] = fs::last_write_time(entry.path());
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
        return current;
    }

    void detect_changes(const FileTimeMap& current) {
        // Check for new or modified files
        for (const auto& [path, time] : current) {
            auto it = snapshot.find(path);
            if (it == snapshot.end()) {
                std::cout << "[NEW] " << path << std::endl;
            } else if (it->second != time) {
                std::cout << "[MODIFIED] " << path << std::endl;
            }
        }

        // Check for deleted files
        for (const auto& [path, time] : snapshot) {
            if (current.find(path) == current.end()) {
                std::cout << "[DELETED] " << path << std::endl;
            }
        }
    }

    std::string watch_path;
    FileTimeMap snapshot;
    bool running;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    std::string path_to_watch = argv[1];
    
    if (!fs::exists(path_to_watch)) {
        std::cerr << "Path does not exist: " << path_to_watch << std::endl;
        return 1;
    }

    SimpleFileWatcher watcher(path_to_watch);
    
    // Run for 30 seconds then stop
    std::thread watch_thread([&watcher]() {
        watcher.start_watching();
    });

    std::this_thread::sleep_for(std::chrono::seconds(30));
    watcher.stop_watching();
    watch_thread.join();

    std::cout << "File watching completed." << std::endl;
    return 0;
}