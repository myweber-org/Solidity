
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <filesystem>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(boost::asio::io_context& io, const std::string& path, std::chrono::seconds interval)
        : timer_(io), watch_path_(path), interval_(interval), last_write_time_() {
        if (fs::exists(watch_path_)) {
            last_write_time_ = fs::last_write_time(watch_path_);
        }
        start_watching();
    }

private:
    void start_watching() {
        timer_.expires_after(interval_);
        timer_.async_wait(boost::bind(&FileSystemWatcher::check_file, this,
                                      boost::asio::placeholders::error));
    }

    void check_file(const boost::system::error_code& ec) {
        if (ec) {
            std::cerr << "Timer error: " << ec.message() << std::endl;
            return;
        }

        if (fs::exists(watch_path_)) {
            auto current_write_time = fs::last_write_time(watch_path_);
            if (current_write_time != last_write_time_) {
                std::cout << "File modified: " << watch_path_ << std::endl;
                last_write_time_ = current_write_time;
            }
        } else {
            std::cout << "File not found: " << watch_path_ << std::endl;
        }

        start_watching();
    }

    boost::asio::steady_timer timer_;
    std::string watch_path_;
    std::chrono::seconds interval_;
    fs::file_time_type last_write_time_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }

    try {
        boost::asio::io_context io;
        FileSystemWatcher watcher(io, argv[1], std::chrono::seconds(2));
        io.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}