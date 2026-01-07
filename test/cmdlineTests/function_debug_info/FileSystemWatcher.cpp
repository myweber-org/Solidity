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
          interval_(interval_seconds) {
        last_write_time_ = get_last_write_time();
        timer_.async_wait(boost::bind(&FileSystemWatcher::check_file, this));
    }

private:
    fs::file_time_type get_last_write_time() {
        if (fs::exists(watch_path_)) {
            return fs::last_write_time(watch_path_);
        }
        return fs::file_time_type::min();
    }

    void check_file() {
        auto current_write_time = get_last_write_time();
        
        if (current_write_time != last_write_time_) {
            if (current_write_time == fs::file_time_type::min()) {
                std::cout << "File deleted: " << watch_path_ << std::endl;
            } else if (last_write_time_ == fs::file_time_type::min()) {
                std::cout << "File created: " << watch_path_ << std::endl;
            } else {
                std::cout << "File modified: " << watch_path_ << std::endl;
            }
            last_write_time_ = current_write_time;
        }

        timer_.expires_at(timer_.expires_at() + boost::posix_time::seconds(interval_));
        timer_.async_wait(boost::bind(&FileSystemWatcher::check_file, this));
    }

    boost::asio::deadline_timer timer_;
    std::string watch_path_;
    int interval_;
    fs::file_time_type last_write_time_;
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
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}