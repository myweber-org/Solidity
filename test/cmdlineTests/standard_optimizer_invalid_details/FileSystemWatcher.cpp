#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path path_to_watch;
    std::unordered_set<std::string> current_files;
    bool running = false;

    std::unordered_set<std::string> get_directory_contents() {
        std::unordered_set<std::string> files;
        if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                files.insert(entry.path().filename().string());
            }
        }
        return files;
    }

    void compare_and_notify(const std::unordered_set<std::string>& old_files,
                            const std::unordered_set<std::string>& new_files) {
        for (const auto& file : new_files) {
            if (old_files.find(file) == old_files.end()) {
                std::cout << "File added: " << file << std::endl;
            }
        }
        for (const auto& file : old_files) {
            if (new_files.find(file) == new_files.end()) {
                std::cout << "File removed: " << file << std::endl;
            }
        }
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        current_files = get_directory_contents();
    }

    void start(int interval_seconds = 2) {
        running = true;
        std::cout << "Watching directory: " << path_to_watch << std::endl;
        
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            
            auto new_files = get_directory_contents();
            compare_and_notify(current_files, new_files);
            current_files = new_files;
        }
    }

    void stop() {
        running = false;
        std::cout << "Stopped watching directory." << std::endl;
    }
};

int main() {
    std::string watch_path = ".";
    FileSystemWatcher watcher(watch_path);
    
    std::thread watch_thread([&watcher]() {
        watcher.start();
    });
    
    std::cout << "Press Enter to stop watching..." << std::endl;
    std::cin.get();
    
    watcher.stop();
    if (watch_thread.joinable()) {
        watch_thread.join();
    }
    
    return 0;
}