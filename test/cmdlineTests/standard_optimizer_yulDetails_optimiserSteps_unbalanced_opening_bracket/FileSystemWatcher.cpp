
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
            build_snapshot();
        }
    }

    void start_watching(int interval_seconds = 1) {
        running = true;
        std::cout << "Watching directory: " << watch_path << std::endl;
        
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

    void stop_watching() {
        running = false;
    }

private:
    void build_snapshot() {
        snapshot.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                auto last_write = fs::last_write_time(entry.path());
                snapshot[entry.path().string()] = last_write;
            }
        }
    }

    void check_for_changes() {
        std::unordered_map<std::string, fs::file_time_type> current_snapshot;
        
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                auto path_str = entry.path().string();
                auto last_write = fs::last_write_time(entry.path());
                current_snapshot[path_str] = last_write;

                auto old_it = snapshot.find(path_str);
                if (old_it == snapshot.end()) {
                    std::cout << "[NEW] " << path_str << std::endl;
                } else if (old_it->second != last_write) {
                    std::cout << "[MODIFIED] " << path_str << std::endl;
                }
            }
        }

        for (const auto& old_entry : snapshot) {
            if (current_snapshot.find(old_entry.first) == current_snapshot.end()) {
                std::cout << "[DELETED] " << old_entry.first << std::endl;
            }
        }

        snapshot = std::move(current_snapshot);
    }

    std::string watch_path;
    std::unordered_map<std::string, fs::file_time_type> snapshot;
    bool running;
};

int main() {
    SimpleFileWatcher watcher(".");
    
    std::thread watch_thread([&watcher]() {
        watcher.start_watching(2);
    });

    std::cout << "File watcher started. Press Enter to stop..." << std::endl;
    std::cin.get();
    
    watcher.stop_watching();
    watch_thread.join();
    
    return 0;
}#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& directory) : watch_directory(directory) {
        if (!fs::exists(watch_directory) || !fs::is_directory(watch_directory)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        update_snapshot();
    }

    void start_watching(int interval_seconds = 2) {
        std::cout << "Watching directory: " << watch_directory.string() << std::endl;
        std::cout << "Press Ctrl+C to stop." << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path watch_directory;
    std::unordered_set<std::string> previous_files;

    void update_snapshot() {
        previous_files.clear();
        for (const auto& entry : fs::directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                previous_files.insert(entry.path().filename().string());
            }
        }
    }

    void check_for_changes() {
        std::unordered_set<std::string> current_files;

        for (const auto& entry : fs::directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                current_files.insert(entry.path().filename().string());
            }
        }

        for (const auto& file : current_files) {
            if (previous_files.find(file) == previous_files.end()) {
                std::cout << "File added: " << file << std::endl;
            }
        }

        for (const auto& file : previous_files) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "File removed: " << file << std::endl;
            }
        }

        previous_files = std::move(current_files);
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start_watching();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}