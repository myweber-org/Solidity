#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileWatcher {
public:
    explicit FileWatcher(const fs::path& path) : watch_path(path) {
        if (!fs::exists(watch_path)) {
            throw std::runtime_error("Path does not exist: " + watch_path.string());
        }
        populate_file_map();
    }

    void start_watching(int interval_seconds = 1) {
        std::cout << "Watching directory: " << watch_path << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_map;

    void populate_file_map() {
        file_map.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                file_map[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void check_for_changes() {
        auto it = file_map.begin();
        while (it != file_map.end()) {
            if (!fs::exists(it->first)) {
                std::cout << "File deleted: " << it->first << std::endl;
                it = file_map.erase(it);
            } else {
                auto current_time = fs::last_write_time(it->first);
                if (current_time != it->second) {
                    std::cout << "File modified: " << it->first << std::endl;
                    it->second = current_time;
                }
                ++it;
            }
        }

        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                std::string path_str = entry.path().string();
                if (file_map.find(path_str) == file_map.end()) {
                    std::cout << "File created: " << path_str << std::endl;
                    file_map[path_str] = fs::last_write_time(entry);
                }
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        FileWatcher watcher(argv[1]);
        watcher.start_watching();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <unordered_set>
#include <atomic>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const fs::path& path) : watch_path(path), running(false) {}

    void start() {
        if (running) return;
        running = true;
        watcher_thread = std::thread(&FileSystemWatcher::watch_loop, this);
    }

    void stop() {
        running = false;
        cv.notify_all();
        if (watcher_thread.joinable()) {
            watcher_thread.join();
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

private:
    void watch_loop() {
        std::unordered_set<std::string> previous_files;

        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                previous_files.insert(entry.path().filename().string());
            }
        }

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            std::unordered_set<std::string> current_files;
            for (const auto& entry : fs::directory_iterator(watch_path)) {
                if (entry.is_regular_file()) {
                    current_files.insert(entry.path().filename().string());
                }
            }

            for (const auto& file : current_files) {
                if (previous_files.find(file) == previous_files.end()) {
                    std::lock_guard<std::mutex> lock(console_mutex);
                    std::cout << "New file detected: " << file << std::endl;
                }
            }

            for (const auto& file : previous_files) {
                if (current_files.find(file) == current_files.end()) {
                    std::lock_guard<std::mutex> lock(console_mutex);
                    std::cout << "File removed: " << file << std::endl;
                }
            }

            previous_files = std::move(current_files);

            std::unique_lock<std::mutex> lock(mtx);
            cv.wait_for(lock, std::chrono::milliseconds(100), [this]() { return !running; });
        }
    }

    fs::path watch_path;
    std::thread watcher_thread;
    std::atomic<bool> running;
    std::mutex mtx;
    std::condition_variable cv;
    std::mutex console_mutex;
};

int main() {
    fs::path current_path = fs::current_path();
    FileSystemWatcher watcher(current_path);

    std::cout << "Watching directory: " << current_path << std::endl;
    std::cout << "Press Enter to stop watching..." << std::endl;

    watcher.start();

    std::cin.get();
    watcher.stop();

    return 0;
}