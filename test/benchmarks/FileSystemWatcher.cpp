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
    FileSystemWatcher(asio::io_context& io, const std::string& path, std::chrono::seconds interval)
        : io_context_(io), timer_(io), watch_path_(path), check_interval_(interval), last_check_time_(fs::last_write_time(path)) {
        start_watching();
    }

    void start_watching() {
        timer_.expires_after(check_interval_);
        timer_.async_wait(boost::bind(&FileSystemWatcher::check_file, this, asio::placeholders::error));
    }

private:
    void check_file(const boost::system::error_code& ec) {
        if (!ec) {
            try {
                if (fs::exists(watch_path_)) {
                    std::time_t current_time = fs::last_write_time(watch_path_);
                    if (current_time != last_check_time_) {
                        last_check_time_ = current_time;
                        std::cout << "File modified: " << watch_path_ << " at " << std::ctime(&current_time);
                        on_file_modified();
                    }
                } else {
                    std::cout << "File removed: " << watch_path_ << std::endl;
                }
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Filesystem error: " << e.what() << std::endl;
            }
            start_watching();
        }
    }

    virtual void on_file_modified() {
        std::cout << "Default handler: File change detected." << std::endl;
    }

    asio::io_context& io_context_;
    asio::steady_timer timer_;
    std::string watch_path_;
    std::chrono::seconds check_interval_;
    std::time_t last_check_time_;
};

class CustomWatcher : public FileSystemWatcher {
public:
    using FileSystemWatcher::FileSystemWatcher;

private:
    void on_file_modified() override {
        std::cout << "Custom handler: Processing modified file..." << std::endl;
    }
};

int main() {
    try {
        asio::io_context io;
        std::string file_to_watch = "test.txt";
        std::chrono::seconds interval(2);

        CustomWatcher watcher(io, file_to_watch, interval);
        std::cout << "Watching file: " << file_to_watch << " (interval: " << interval.count() << "s)" << std::endl;

        io.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}