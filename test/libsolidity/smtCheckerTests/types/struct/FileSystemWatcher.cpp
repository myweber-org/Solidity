
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace fs = boost::filesystem;
namespace asio = boost::asio;

class FileSystemWatcher {
public:
    FileSystemWatcher(asio::io_context& io, const std::string& path, std::chrono::milliseconds interval)
        : timer_(io), watch_path_(path), interval_(interval), last_check_time_(fs::last_write_time(path)) {
        start_watching();
    }

private:
    void start_watching() {
        timer_.expires_from_now(boost::posix_time::milliseconds(interval_.count()));
        timer_.async_wait(boost::bind(&FileSystemWatcher::check_file, this, asio::placeholders::error));
    }

    void check_file(const boost::system::error_code& ec) {
        if (!ec) {
            try {
                if (fs::exists(watch_path_)) {
                    std::time_t current_time = fs::last_write_time(watch_path_);
                    if (current_time != last_check_time_) {
                        last_check_time_ = current_time;
                        std::cout << "File modified: " << watch_path_ << std::endl;
                        // Trigger custom callback here if needed
                    }
                } else {
                    std::cout << "File deleted: " << watch_path_ << std::endl;
                }
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Filesystem error: " << e.what() << std::endl;
            }
            start_watching();
        }
    }

    asio::deadline_timer timer_;
    std::string watch_path_;
    std::chrono::milliseconds interval_;
    std::time_t last_check_time_;
};

int main() {
    try {
        asio::io_context io;
        std::string file_to_watch = "test_file.txt";
        FileSystemWatcher watcher(io, file_to_watch, std::chrono::seconds(1));
        io.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}