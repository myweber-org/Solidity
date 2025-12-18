
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <thread>
#include <chrono>

namespace fs = boost::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(boost::asio::io_context& io, const std::string& path)
        : timer_(io), watch_path_(path) {
        last_write_time_ = get_last_write_time();
        start_timer();
    }

private:
    void start_timer() {
        timer_.expires_after(std::chrono::seconds(1));
        timer_.async_wait([this](const boost::system::error_code& ec) {
            if (!ec) {
                check_file_changes();
                start_timer();
            }
        });
    }

    std::time_t get_last_write_time() {
        if (fs::exists(watch_path_)) {
            return fs::last_write_time(watch_path_);
        }
        return 0;
    }

    void check_file_changes() {
        auto current_time = get_last_write_time();
        if (current_time != last_write_time_) {
            std::cout << "File modified: " << watch_path_ << std::endl;
            last_write_time_ = current_time;
        }
    }

    boost::asio::steady_timer timer_;
    std::string watch_path_;
    std::time_t last_write_time_;
};

int main() {
    try {
        boost::asio::io_context io;
        FileSystemWatcher watcher(io, "example.txt");
        
        std::thread io_thread([&io]() { io.run(); });
        
        std::cout << "Watching for file changes... Press Enter to exit." << std::endl;
        std::cin.get();
        
        io.stop();
        io_thread.join();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}