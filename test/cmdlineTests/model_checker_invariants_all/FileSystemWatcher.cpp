
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

    void compare_and_notify(const std::unordered_set<std::string>& new_files) {
        // Check for added files
        for (const auto& file : new_files) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "File added: " << file << std::endl;
            }
        }

        // Check for removed files
        for (const auto& file : current_files) {
            if (new_files.find(file) == new_files.end()) {
                std::cout << "File removed: " << file << std::endl;
            }
        }
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        current_files = get_directory_contents();
    }

    void start(int interval_seconds = 1) {
        running = true;
        std::cout << "Watching directory: " << path_to_watch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            
            auto new_files = get_directory_contents();
            compare_and_notify(new_files);
            current_files = new_files;
        }
    }

    void stop() {
        running = false;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    FileSystemWatcher watcher(argv[1]);
    
    // Handle Ctrl+C or other termination signals in a simple way
    std::thread watch_thread([&watcher]() {
        watcher.start();
    });

    std::cout << "Press Enter to stop watching..." << std::endl;
    std::cin.get();
    
    watcher.stop();
    watch_thread.join();
    
    return 0;
}