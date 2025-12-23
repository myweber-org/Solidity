
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

    void start(int interval_seconds = 1) {
        running = true;
        std::cout << "Watching directory: " << path_to_wick.string() << std::endl;
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            auto new_files = get_directory_contents();
            compare_and_notify(current_files, new_files);
            current_files = new_files;
        }
    }

    void stop() {
        running = false;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    std::string watch_path = argv[1];
    FileSystemWatcher watcher(watch_path);

    watcher.start(2);

    return 0;
}