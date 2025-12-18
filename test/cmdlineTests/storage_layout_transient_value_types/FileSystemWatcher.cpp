
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
        start_watch();
    }

    ~FileSystemWatcher() {
        timer_.cancel();
    }

private:
    void start_watch() {
        last_check_time_ = fs::last_write_time(watch_path_);
        timer_.async_wait(boost::bind(&FileSystemWatcher::check_file, this));
    }

    void check_file() {
        try {
            auto current_time = fs::last_write_time(watch_path_);
            if (current_time != last_check_time_) {
                std::cout << "File modified: " << watch_path_ << std::endl;
                last_check_time_ = current_time;
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }

        timer_.expires_at(timer_.expires_at() + boost::posix_time::seconds(interval_seconds_));
        timer_.async_wait(boost::bind(&FileSystemWatcher::check_file, this));
    }

    boost::asio::deadline_timer timer_;
    std::string watch_path_;
    int interval_seconds_;
    fs::file_time_type last_check_time_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }

    try {
        boost::asio::io_context io;
        FileSystemWatcher watcher(io, argv[1]);
        io.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}