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
        last_write_time_ = get_last_write_time(watch_path_);
        start_watch();
    }

    ~FileSystemWatcher() {
        timer_.cancel();
    }

private:
    void start_watch() {
        timer_.async_wait(boost::bind(&FileSystemWatcher::check_file, this));
    }

    void check_file() {
        try {
            auto current_write_time = get_last_write_time(watch_path_);
            if (current_write_time != last_write_time_) {
                std::cout << "File change detected at: " << watch_path_ << std::endl;
                last_write_time_ = current_write_time;
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }

        timer_.expires_at(timer_.expires_at() + boost::posix_time::seconds(interval_));
        start_watch();
    }

    std::time_t get_last_write_time(const std::string& path) {
        auto ftime = fs::last_write_time(path);
        return decltype(ftime)::clock::to_time_t(ftime);
    }

    boost::asio::deadline_timer timer_;
    std::string watch_path_;
    int interval_;
    std::time_t last_write_time_;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path_to_watch>" << std::endl;
        return 1;
    }

    try {
        boost::asio::io_context io;
        FileSystemWatcher watcher(io, argv[1], 2);

        std::thread io_thread([&io]() { io.run(); });

        std::cout << "Watching file: " << argv[1] << " (Press Enter to stop)" << std::endl;
        std::cin.get();

        io.stop();
        io_thread.join();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}