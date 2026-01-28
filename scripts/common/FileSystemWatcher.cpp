
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
        : timer_(io), watch_path_(path), last_check_(fs::file_time_type::min()) {
        start_watching();
    }

private:
    void start_watching() {
        timer_.expires_after(std::chrono::seconds(1));
        timer_.async_wait(boost::bind(&FileSystemWatcher::check_changes, this));
    }

    void check_changes() {
        try {
            auto new_check_time = fs::last_write_time(watch_path_);
            if (new_check_time != last_check_) {
                last_check_ = new_check_time;
                std::cout << "File modified: " << watch_path_ << std::endl;
            }
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
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }

    boost::asio::io_context io;
    FileSystemWatcher watcher(io, argv[1]);

    std::thread io_thread([&io]() { io.run(); });

    std::cout << "Watching for changes in: " << argv[1] << std::endl;
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    io.stop();
    if (io_thread.joinable()) {
        io_thread.join();
    }

    return 0;
}