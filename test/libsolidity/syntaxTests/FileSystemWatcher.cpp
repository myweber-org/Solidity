
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace fs = boost::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(boost::asio::io_context& io, const std::string& path)
        : io_context_(io), timer_(io), watch_path_(path), last_check_time_(boost::posix_time::second_clock::local_time()) {
        start_watch();
    }

private:
    void start_watch() {
        timer_.expires_from_now(boost::posix_time::seconds(1));
        timer_.async_wait(boost::bind(&FileSystemWatcher::check_files, this));
    }

    void check_files() {
        boost::posix_time::ptime current_time = boost::posix_time::second_clock::local_time();
        try {
            if (fs::exists(watch_path_) && fs::is_directory(watch_path_)) {
                fs::directory_iterator end_iter;
                for (fs::directory_iterator dir_iter(watch_path_); dir_iter != end_iter; ++dir_iter) {
                    if (fs::is_regular_file(dir_iter->status())) {
                        fs::file_time_type file_time = fs::last_write_time(dir_iter->path());
                        auto duration = file_time - last_check_time_;
                        if (duration.total_seconds() > 0) {
                            std::cout << "File modified: " << dir_iter->path().string() << std::endl;
                        }
                    }
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
        last_check_time_ = current_time;
        start_watch();
    }

    boost::asio::io_context& io_context_;
    boost::asio::deadline_timer timer_;
    std::string watch_path_;
    boost::posix_time::ptime last_check_time_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
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