#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const fs::path& path) : watch_path(path) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        last_write_time = getLastWriteTime();
    }

    void startMonitoring(int interval_seconds = 1) {
        std::cout << "Starting to monitor: " << watch_path.string() << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            checkForChanges();
        }
    }

private:
    fs::path watch_path;
    std::time_t last_write_time;

    std::time_t getLastWriteTime() {
        auto ftime = fs::last_write_time(watch_path);
        return decltype(ftime)::clock::to_time_t(ftime);
    }

    void checkForChanges() {
        std::time_t current_time = getLastWriteTime();
        if (current_time != last_write_time) {
            last_write_time = current_time;
            std::cout << "Change detected in: " << watch_path.string() << " at "
                      << std::ctime(&current_time);
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.startMonitoring();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}