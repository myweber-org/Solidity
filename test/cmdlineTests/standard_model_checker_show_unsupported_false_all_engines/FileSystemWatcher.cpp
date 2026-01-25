
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind/bind.hpp>

namespace fs = boost::filesystem;
namespace asio = boost::asio;

class FileSystemWatcher {
public:
    FileSystemWatcher(asio::io_context& io_context, const std::string& path)
        : timer_(io_context), watch_path_(path), running_(false) {
        if (fs::exists(watch_path_) && fs::is_directory(watch_path_)) {
            last_write_time_ = fs::last_write_time(watch_path_);
        }
    }

    void start() {
        running_ = true;
        schedule_check();
    }

    void stop() {
        running_ = false;
        timer_.cancel();
    }

private:
    void schedule_check() {
        if (!running_) return;

        timer_.expires_after(std::chrono::seconds(1));
        timer_.async_wait([this](const boost::system::error_code& ec) {
            if (!ec && running_) {
                check_for_changes();
                schedule_check();
            }
        });
    }

    void check_for_changes() {
        try {
            if (!fs::exists(watch_path_)) {
                std::cout << "Watch path does not exist: " << watch_path_ << std::endl;
                return;
            }

            auto current_time = fs::last_write_time(watch_path_);
            if (current_time != last_write_time_) {
                last_write_time_ = current_time;
                std::cout << "Directory modified: " << watch_path_ << std::endl;
                scan_directory();
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
    }

    void scan_directory() {
        try {
            for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
                if (fs::is_regular_file(entry.path())) {
                    std::cout << "  File: " << entry.path().string() << std::endl;
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Scan error: " << e.what() << std::endl;
        }
    }

    asio::steady_timer timer_;
    std::string watch_path_;
    std::time_t last_write_time_;
    bool running_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        asio::io_context io_context;
        FileSystemWatcher watcher(io_context, argv[1]);

        std::cout << "Starting file system watcher for: " << argv[1] << std::endl;
        std::cout << "Press Ctrl+C to stop..." << std::endl;

        watcher.start();

        std::thread io_thread([&io_context]() {
            io_context.run();
        });

        io_thread.join();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}