
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
    FileSystemWatcher(asio::io_context& io, const std::string& path)
        : timer_(io), watch_path_(path), running_(false) {
        if (!fs::exists(watch_path_) || !fs::is_directory(watch_path_)) {
            throw std::runtime_error("Invalid directory path: " + watch_path_);
        }
        cache_directory_state();
    }

    void start() {
        if (running_) return;
        running_ = true;
        schedule_check();
    }

    void stop() {
        running_ = false;
        timer_.cancel();
    }

private:
    void cache_directory_state() {
        file_cache_.clear();
        for (fs::directory_iterator it(watch_path_); it != fs::directory_iterator(); ++it) {
            if (fs::is_regular_file(*it)) {
                file_cache_[it->path().filename().string()] = fs::last_write_time(*it);
            }
        }
    }

    void schedule_check() {
        if (!running_) return;
        timer_.expires_after(std::chrono::seconds(2));
        timer_.async_wait([this](const boost::system::error_code& ec) {
            if (!ec && running_) {
                check_for_changes();
                schedule_check();
            }
        });
    }

    void check_for_changes() {
        std::map<std::string, fs::file_time_type> current_state;
        for (fs::directory_iterator it(watch_path_); it != fs::directory_iterator(); ++it) {
            if (fs::is_regular_file(*it)) {
                current_state[it->path().filename().string()] = fs::last_write_time(*it);
            }
        }

        for (const auto& entry : current_state) {
            auto old_it = file_cache_.find(entry.first);
            if (old_it == file_cache_.end()) {
                std::cout << "[NEW] File detected: " << entry.first << std::endl;
            } else if (old_it->second != entry.second) {
                std::cout << "[MODIFIED] File changed: " << entry.first << std::endl;
            }
        }

        for (const auto& entry : file_cache_) {
            if (current_state.find(entry.first) == current_state.end()) {
                std::cout << "[DELETED] File removed: " << entry.first << std::endl;
            }
        }

        file_cache_.swap(current_state);
    }

    asio::steady_timer timer_;
    std::string watch_path_;
    std::map<std::string, fs::file_time_type> file_cache_;
    bool running_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        asio::io_context io;
        FileSystemWatcher watcher(io, argv[1]);
        watcher.start();

        std::cout << "Watching directory: " << argv[1] << std::endl;
        std::cout << "Press Ctrl+C to stop..." << std::endl;

        io.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}