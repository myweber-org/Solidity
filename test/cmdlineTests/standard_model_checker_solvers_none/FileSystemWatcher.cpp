
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

    void populate_file_set() {
        current_files.clear();
        for (const auto& entry : fs::directory_iterator(path_to_watch)) {
            current_files.insert(entry.path().filename().string());
        }
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
            throw std::invalid_argument("Path does not exist or is not a directory");
        }
        populate_file_set();
    }

    void start(int interval_seconds = 1) {
        running = true;
        std::cout << "Watching directory: " << path_to_watch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

    void stop() {
        running = false;
    }

private:
    void check_for_changes() {
        auto new_files = std::unordered_set<std::string>();
        bool changed = false;

        for (const auto& entry : fs::directory_iterator(path_to_watch)) {
            std::string filename = entry.path().filename().string();
            new_files.insert(filename);

            if (current_files.find(filename) == current_files.end()) {
                std::cout << "File added: " << filename << std::endl;
                changed = true;
            }
        }

        for (const auto& old_file : current_files) {
            if (new_files.find(old_file) == new_files.end()) {
                std::cout << "File removed: " << old_file << std::endl;
                changed = true;
            }
        }

        if (changed) {
            current_files = std::move(new_files);
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
        watcher.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
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
    FileSystemWatcher(boost::asio::io_context& io, const std::string& path, int interval_seconds = 1)
        : timer_(io, boost::posix_time::seconds(interval_seconds)),
          watch_path_(path),
          interval_seconds_(interval_seconds) {
        last_write_time_ = get_last_write_time();
        timer_.async_wait(boost::bind(&FileSystemWatcher::check_file, this));
    }

private:
    fs::file_time_type get_last_write_time() {
        try {
            if (fs::exists(watch_path_)) {
                return fs::last_write_time(watch_path_);
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
        return fs::file_time_type::min();
    }

    void check_file() {
        auto current_write_time = get_last_write_time();
        
        if (current_write_time != last_write_time_) {
            if (current_write_time == fs::file_time_type::min()) {
                std::cout << "File deleted or inaccessible: " << watch_path_ << std::endl;
            } else if (last_write_time_ == fs::file_time_type::min()) {
                std::cout << "File created: " << watch_path_ << std::endl;
            } else {
                std::cout << "File modified: " << watch_path_ << std::endl;
            }
            last_write_time_ = current_write_time;
        }

        timer_.expires_at(timer_.expires_at() + boost::posix_time::seconds(interval_seconds_));
        timer_.async_wait(boost::bind(&FileSystemWatcher::check_file, this));
    }

    boost::asio::deadline_timer timer_;
    std::string watch_path_;
    int interval_seconds_;
    fs::file_time_type last_write_time_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }

    try {
        boost::asio::io_context io;
        FileSystemWatcher watcher(io, argv[1], 2);
        
        std::cout << "Watching file: " << argv[1] << std::endl;
        std::cout << "Press Ctrl+C to stop" << std::endl;
        
        io.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}