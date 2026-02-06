
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <filesystem>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(boost::asio::io_context& io, const std::string& path)
        : timer_(io), watch_path_(path), running_(false) {
        if (!fs::exists(watch_path_) || !fs::is_directory(watch_path_)) {
            throw std::runtime_error("Invalid directory path");
        }
    }

    void start() {
        running_ = true;
        last_check_time_ = fs::last_write_time(watch_path_);
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
        timer_.async_wait(boost::bind(&FileSystemWatcher::check_directory, this,
                                     boost::asio::placeholders::error));
    }

    void check_directory(const boost::system::error_code& ec) {
        if (ec || !running_) return;

        try {
            auto current_time = fs::last_write_time(watch_path_);
            if (current_time != last_check_time_) {
                last_check_time_ = current_time;
                std::cout << "Directory modified: " << watch_path_ << std::endl;
                
                for (const auto& entry : fs::directory_iterator(watch_path_)) {
                    std::cout << "  Found: " << entry.path().filename() << std::endl;
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }

        schedule_check();
    }

    boost::asio::steady_timer timer_;
    std::string watch_path_;
    fs::file_time_type last_check_time_;
    bool running_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        boost::asio::io_context io;
        FileSystemWatcher watcher(io, argv[1]);
        
        watcher.start();
        
        std::thread io_thread([&io]() { io.run(); });
        
        std::cout << "Watching directory: " << argv[1] << std::endl;
        std::cout << "Press Enter to stop..." << std::endl;
        std::cin.get();
        
        watcher.stop();
        io.stop();
        io_thread.join();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}