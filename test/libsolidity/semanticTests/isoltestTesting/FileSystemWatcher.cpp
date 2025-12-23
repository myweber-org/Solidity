
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