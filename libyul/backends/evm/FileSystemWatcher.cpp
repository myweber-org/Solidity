
#include <iostream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind/bind.hpp>
#include <set>

namespace fs = boost::filesystem;
namespace asio = boost::asio;

class FileSystemWatcher {
public:
    FileSystemWatcher(asio::io_context& io, const std::string& path)
        : timer_(io), watch_path_(path), last_check_(std::chrono::steady_clock::now()) {
        scan_files();
        start_timer();
    }

private:
    void start_timer() {
        timer_.expires_after(std::chrono::seconds(1));
        timer_.async_wait([this](const boost::system::error_code& ec) {
            if (!ec) {
                check_for_changes();
                start_timer();
            }
        });
    }

    void scan_files() {
        current_files_.clear();
        if (fs::exists(watch_path_) && fs::is_directory(watch_path_)) {
            for (const auto& entry : fs::directory_iterator(watch_path_)) {
                if (fs::is_regular_file(entry.path())) {
                    current_files_.insert(entry.path().string());
                }
            }
        }
    }

    void check_for_changes() {
        auto now = std::chrono::steady_clock::now();
        if (now - last_check_ < std::chrono::seconds(1)) return;
        last_check_ = now;

        std::set<std::string> new_files;
        if (fs::exists(watch_path_) && fs::is_directory(watch_path_)) {
            for (const auto& entry : fs::directory_iterator(watch_path_)) {
                if (fs::is_regular_file(entry.path())) {
                    std::string path = entry.path().string();
                    new_files.insert(path);
                    if (current_files_.find(path) == current_files_.end()) {
                        std::cout << "File added: " << path << std::endl;
                    }
                }
            }
        }

        for (const auto& old_file : current_files_) {
            if (new_files.find(old_file) == new_files.end()) {
                std::cout << "File removed: " << old_file << std::endl;
            }
        }

        current_files_.swap(new_files);
    }

    asio::steady_timer timer_;
    std::string watch_path_;
    std::set<std::string> current_files_;
    std::chrono::steady_clock::time_point last_check_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        asio::io_context io;
        FileSystemWatcher watcher(io, argv[1]);
        io.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}