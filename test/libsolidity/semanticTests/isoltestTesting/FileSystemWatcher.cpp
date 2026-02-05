
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <filesystem>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(boost::asio::io_service& io, const std::string& path, int interval_seconds = 1)
        : timer_(io, boost::posix_time::seconds(interval_seconds)),
          watch_path_(path),
          interval_(interval_seconds) {
        // Store initial state
        update_file_map();
        // Start the periodic check
        timer_.async_wait(boost::bind(&FileSystemWatcher::check_files, this));
    }

    void update_file_map() {
        file_map_.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (fs::is_regular_file(entry.path())) {
                auto last_write = fs::last_write_time(entry.path());
                file_map_[entry.path().string()] = last_write;
            }
        }
    }

    void check_files() {
        // Check for new or modified files
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (fs::is_regular_file(entry.path())) {
                std::string path_str = entry.path().string();
                auto current_time = fs::last_write_time(entry.path());

                if (file_map_.find(path_str) == file_map_.end()) {
                    std::cout << "[NEW] " << path_str << std::endl;
                    file_map_[path_str] = current_time;
                } else if (file_map_[path_str] != current_time) {
                    std::cout << "[MODIFIED] " << path_str << std::endl;
                    file_map_[path_str] = current_time;
                }
            }
        }

        // Check for deleted files
        auto it = file_map_.begin();
        while (it != file_map_.end()) {
            if (!fs::exists(it->first)) {
                std::cout << "[DELETED] " << it->first << std::endl;
                it = file_map_.erase(it);
            } else {
                ++it;
            }
        }

        // Reset timer
        timer_.expires_at(timer_.expires_at() + boost::posix_time::seconds(interval_));
        timer_.async_wait(boost::bind(&FileSystemWatcher::check_files, this));
    }

private:
    boost::asio::deadline_timer timer_;
    std::string watch_path_;
    int interval_;
    std::unordered_map<std::string, fs::file_time_type> file_map_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    std::string watch_directory = argv[1];
    if (!fs::exists(watch_directory) || !fs::is_directory(watch_directory)) {
        std::cerr << "Error: " << watch_directory << " is not a valid directory." << std::endl;
        return 1;
    }

    try {
        boost::asio::io_service io;
        FileSystemWatcher watcher(io, watch_directory, 2); // Check every 2 seconds

        std::cout << "Watching directory: " << watch_directory << std::endl;
        std::cout << "Press Ctrl+C to stop..." << std::endl;

        io.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const std::string& path) : watch_path(path) {
        if (fs::exists(path) && fs::is_directory(path)) {
            populate_snapshot();
        }
    }

    void start_watching(int interval_seconds = 2) {
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
    std::string watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_snapshot;
    bool running = true;

    void populate_snapshot() {
        file_snapshot.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_snapshot[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void check_for_changes() {
        std::unordered_map<std::string, fs::file_time_type> current_state;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                std::string file_path = entry.path().string();
                auto last_write = fs::last_write_time(entry.path());
                current_state[file_path] = last_write;

                auto it = file_snapshot.find(file_path);
                if (it == file_snapshot.end()) {
                    std::cout << "[ADDED] " << file_path << std::endl;
                } else if (it->second != last_write) {
                    std::cout << "[MODIFIED] " << file_path << std::endl;
                }
            }
        }

        for (const auto& [old_file, _] : file_snapshot) {
            if (current_state.find(old_file) == current_state.end()) {
                std::cout << "[DELETED] " << old_file << std::endl;
            }
        }

        file_snapshot = std::move(current_state);
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    std::string path_to_watch = argv[1];
    FileSystemWatcher watcher(path_to_watch);

    watcher.start_watching();

    return 0;
}