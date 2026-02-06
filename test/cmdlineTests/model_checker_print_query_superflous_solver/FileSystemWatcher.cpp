#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path directory_to_watch;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps;
    bool running = false;

    void populate_file_map() {
        file_timestamps.clear();
        for (const auto& entry : fs::directory_iterator(directory_to_watch)) {
            if (fs::is_regular_file(entry.status())) {
                file_timestamps[entry.path().filename().string()] = fs::last_write_time(entry);
            }
        }
    }

public:
    explicit FileSystemWatcher(const std::string& dir_path) : directory_to_watch(dir_path) {
        if (!fs::exists(directory_to_watch) || !fs::is_directory(directory_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        populate_file_map();
        std::cout << "Watching directory: " << fs::absolute(directory_to_watch) << std::endl;
    }

    void start(int interval_seconds = 1) {
        running = true;
        std::cout << "Starting watcher. Checking every " << interval_seconds << " second(s)." << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            for (const auto& entry : fs::directory_iterator(directory_to_watch)) {
                if (!fs::is_regular_file(entry.status())) {
                    continue;
                }

                std::string filename = entry.path().filename().string();
                auto current_write_time = fs::last_write_time(entry);

                if (file_timestamps.find(filename) == file_timestamps.end()) {
                    std::cout << "[NEW] File created: " << filename << std::endl;
                    file_timestamps[filename] = current_write_time;
                } else if (file_timestamps[filename] != current_write_time) {
                    std::cout << "[MODIFIED] File changed: " << filename << std::endl;
                    file_timestamps[filename] = current_write_time;
                }
            }

            for (auto it = file_timestamps.begin(); it != file_timestamps.end(); ) {
                std::string old_filename = it->first;
                fs::path old_file_path = directory_to_watch / old_filename;

                if (!fs::exists(old_file_path)) {
                    std::cout << "[DELETED] File removed: " << old_filename << std::endl;
                    it = file_timestamps.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    void stop() {
        running = false;
        std::cout << "Watcher stopped." << std::endl;
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start(2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path directory_path;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps;

    void populate_timestamps() {
        file_timestamps.clear();
        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (entry.is_regular_file()) {
                file_timestamps[entry.path().filename().string()] = fs::last_write_time(entry);
            }
        }
    }

public:
    FileSystemWatcher(const std::string& path) : directory_path(path) {
        if (!fs::exists(directory_path) || !fs::is_directory(directory_path)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        populate_timestamps();
        std::cout << "Watching directory: " << directory_path << std::endl;
    }

    void check_for_changes() {
        auto current_timestamps = file_timestamps;
        populate_timestamps();

        for (const auto& [filename, old_time] : current_timestamps) {
            auto it = file_timestamps.find(filename);
            if (it == file_timestamps.end()) {
                std::cout << "File removed: " << filename << std::endl;
            } else if (it->second != old_time) {
                std::cout << "File modified: " << filename << std::endl;
            }
        }

        for (const auto& [filename, new_time] : file_timestamps) {
            if (current_timestamps.find(filename) == current_timestamps.end()) {
                std::cout << "File created: " << filename << std::endl;
            }
        }
    }

    void run(int interval_seconds = 2) {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            try {
                check_for_changes();
            } catch (const std::exception& e) {
                std::cerr << "Error checking directory: " << e.what() << std::endl;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.run();
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize watcher: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}