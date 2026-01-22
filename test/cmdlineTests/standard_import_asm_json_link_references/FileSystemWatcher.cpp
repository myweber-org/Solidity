
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
    FileSystemWatcher(boost::asio::io_context& io, const std::string& path)
        : timer_(io), watch_path_(path), last_check_(fs::file_time_type::clock::now()) {
        start_watching();
    }

private:
    void start_watching() {
        timer_.expires_after(std::chrono::seconds(1));
        timer_.async_wait(boost::bind(&FileSystemWatcher::check_changes, this,
                                      boost::asio::placeholders::error));
    }

    void check_changes(const boost::system::error_code& ec) {
        if (ec) {
            std::cerr << "Timer error: " << ec.message() << std::endl;
            return;
        }

        try {
            auto current_time = fs::file_time_type::clock::now();
            for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
                if (fs::is_regular_file(entry.status())) {
                    auto write_time = fs::last_write_time(entry.path());
                    if (write_time > last_check_) {
                        std::cout << "File modified: " << entry.path().string() << std::endl;
                    }
                }
            }
            last_check_ = current_time;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }

        start_watching();
    }

    boost::asio::steady_timer timer_;
    std::string watch_path_;
    fs::file_time_type last_check_;
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
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}