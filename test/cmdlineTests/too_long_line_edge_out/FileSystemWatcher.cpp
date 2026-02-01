
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <unordered_map>

namespace fs = boost::filesystem;
namespace asio = boost::asio;

class FileSystemWatcher {
public:
    FileSystemWatcher(asio::io_service& io_service, const std::string& path)
        : timer_(io_service), watch_path_(path), running_(false) {
        scan_files();
    }

    void start() {
        running_ = true;
        schedule_timer();
    }

    void stop() {
        running_ = false;
        timer_.cancel();
    }

private:
    void schedule_timer() {
        timer_.expires_from_now(boost::posix_time::seconds(1));
        timer_.async_wait(boost::bind(&FileSystemWatcher::check_files, this, asio::placeholders::error));
    }

    void scan_files() {
        file_states_.clear();
        if (fs::exists(watch_path_) && fs::is_directory(watch_path_)) {
            for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
                if (fs::is_regular_file(entry.path())) {
                    file_states_[entry.path().string()] = fs::last_write_time(entry.path());
                }
            }
        }
    }

    void check_files(const boost::system::error_code& ec) {
        if (ec || !running_) return;

        std::unordered_map<std::string, fs::file_time_type> current_states;
        
        if (fs::exists(watch_path_) && fs::is_directory(watch_path_)) {
            for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
                if (fs::is_regular_file(entry.path())) {
                    std::string path = entry.path().string();
                    current_states[path] = fs::last_write_time(entry.path());
                    
                    auto it = file_states_.find(path);
                    if (it == file_states_.end()) {
                        std::cout << "File created: " << path << std::endl;
                    } else if (it->second != current_states[path]) {
                        std::cout << "File modified: " << path << std::endl;
                    }
                }
            }
        }

        for (const auto& old_file : file_states_) {
            if (current_states.find(old_file.first) == current_states.end()) {
                std::cout << "File deleted: " << old_file.first << std::endl;
            }
        }

        file_states_.swap(current_states);
        schedule_timer();
    }

    asio::deadline_timer timer_;
    std::string watch_path_;
    std::unordered_map<std::string, fs::file_time_type> file_states_;
    bool running_;
};

int main() {
    asio::io_service io_service;
    FileSystemWatcher watcher(io_service, ".");
    
    watcher.start();
    
    std::thread io_thread([&io_service]() {
        io_service.run();
    });
    
    std::this_thread::sleep_for(std::chrono::seconds(30));
    
    watcher.stop();
    io_service.stop();
    io_thread.join();
    
    return 0;
}