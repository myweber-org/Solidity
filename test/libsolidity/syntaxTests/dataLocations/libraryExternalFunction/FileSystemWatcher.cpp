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
        : timer_(io_service), watch_path_(path), last_check_time_(boost::posix_time::microsec_clock::local_time()) {
        start_watch();
    }

private:
    void start_watch() {
        timer_.expires_from_now(boost::posix_time::seconds(1));
        timer_.async_wait(boost::bind(&FileSystemWatcher::check_files, this, asio::placeholders::error));
    }

    void check_files(const boost::system::error_code& e) {
        if (e) {
            std::cerr << "Timer error: " << e.message() << std::endl;
            return;
        }

        try {
            if (!fs::exists(watch_path_) || !fs::is_directory(watch_path_)) {
                std::cerr << "Watch path is not a valid directory: " << watch_path_ << std::endl;
                return;
            }

            boost::posix_time::ptime current_time = boost::posix_time::microsec_clock::local_time();
            for (fs::directory_iterator it(watch_path_); it != fs::directory_iterator(); ++it) {
                if (fs::is_regular_file(it->status())) {
                    std::time_t last_write = fs::last_write_time(it->path());
                    boost::posix_time::ptime file_time = boost::posix_time::from_time_t(last_write);

                    if (file_time > last_check_time_) {
                        std::cout << "File modified: " << it->path().filename().string()
                                  << " at " << boost::posix_time::to_simple_string(file_time) << std::endl;
                    }
                }
            }
            last_check_time_ = current_time;
        } catch (const fs::filesystem_error& ex) {
            std::cerr << "Filesystem error: " << ex.what() << std::endl;
        }

        start_watch();
    }

    asio::deadline_timer timer_;
    std::string watch_path_;
    boost::posix_time::ptime last_check_time_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        asio::io_service io_service;
        FileSystemWatcher watcher(io_service, argv[1]);
        io_service.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}