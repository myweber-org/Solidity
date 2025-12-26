#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind/bind.hpp>
#include <boost/system/error_code.hpp>

namespace fs = boost::filesystem;
namespace asio = boost::asio;

class FileSystemWatcher {
public:
    FileSystemWatcher(asio::io_context& io, const std::string& path, std::chrono::milliseconds interval)
        : timer_(io), watch_path_(path), interval_(interval), last_write_time_(fs::last_write_time(path)) {
        start_watching();
    }

private:
    void start_watching() {
        timer_.expires_after(interval_);
        timer_.async_wait(boost::bind(&FileSystemWatcher::check_file, this, asio::placeholders::error));
    }

    void check_file(const boost::system::error_code& ec) {
        if (!ec) {
            try {
                std::time_t current_write_time = fs::last_write_time(watch_path_);
                if (current_write_time != last_write_time_) {
                    std::cout << "File modified: " << watch_path_ << std::endl;
                    last_write_time_ = current_write_time;
                }
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Filesystem error: " << e.what() << std::endl;
            }
            start_watching();
        }
    }

    asio::steady_timer timer_;
    std::string watch_path_;
    std::chrono::milliseconds interval_;
    std::time_t last_write_time_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }

    try {
        asio::io_context io;
        FileSystemWatcher watcher(io, argv[1], std::chrono::milliseconds(500));
        io.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}