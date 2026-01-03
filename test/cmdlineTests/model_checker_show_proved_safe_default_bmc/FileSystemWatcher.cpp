
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
    FileSystemWatcher(asio::io_service& io_service, const std::string& path)
        : timer_(io_service), watch_path_(path), last_check_(fs::last_write_time(path)) {
        start_timer();
    }

    void start_timer() {
        timer_.expires_from_now(boost::posix_time::seconds(1));
        timer_.async_wait(boost::bind(&FileSystemWatcher::check_file, this,
                                      asio::placeholders::error));
    }

    void check_file(const boost::system::error_code& e) {
        if (e) {
            std::cerr << "Timer error: " << e.message() << std::endl;
            return;
        }

        try {
            if (!fs::exists(watch_path_)) {
                std::cout << "File deleted: " << watch_path_ << std::endl;
                return;
            }

            std::time_t current_time = fs::last_write_time(watch_path_);
            if (current_time != last_check_) {
                std::cout << "File modified: " << watch_path_ << std::endl;
                last_check_ = current_time;
            }
        } catch (const fs::filesystem_error& ex) {
            std::cerr << "Filesystem error: " << ex.what() << std::endl;
        }

        start_timer();
    }

private:
    asio::deadline_timer timer_;
    std::string watch_path_;
    std::time_t last_check_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }

    try {
        asio::io_service io_service;
        FileSystemWatcher watcher(io_service, argv[1]);
        io_service.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}