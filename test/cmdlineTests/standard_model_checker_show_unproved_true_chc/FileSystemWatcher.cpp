
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
    FileSystemWatcher(asio::io_context& io, const std::string& path)
        : timer_(io), watch_path_(path), last_check_(fs::last_write_time(path)) {
        start_watching();
    }

private:
    void start_watching() {
        timer_.expires_after(std::chrono::seconds(1));
        timer_.async_wait(boost::bind(&FileSystemWatcher::check_file, this,
                                      asio::placeholders::error));
    }

    void check_file(const boost::system::error_code& ec) {
        if (ec) {
            std::cerr << "Timer error: " << ec.message() << std::endl;
            return;
        }

        try {
            if (!fs::exists(watch_path_)) {
                std::cout << "File deleted: " << watch_path_ << std::endl;
                return;
            }

            auto current_time = fs::last_write_time(watch_path_);
            if (current_time != last_check_) {
                std::cout << "File modified: " << watch_path_ << std::endl;
                last_check_ = current_time;
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }

        start_watching();
    }

    asio::steady_timer timer_;
    std::string watch_path_;
    std::time_t last_check_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }

    try {
        asio::io_context io;
        FileSystemWatcher watcher(io, argv[1]);
        io.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}