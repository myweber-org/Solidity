
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
                if (fs::is_regular_file(entry.status())) {
                    current_files_.insert(entry.path().string());
                }
            }
        }
    }

    void check_for_changes() {
        auto old_files = current_files_;
        scan_files();

        std::set<std::string> added_files;
        std::set_difference(current_files_.begin(), current_files_.end(),
                            old_files.begin(), old_files.end(),
                            std::inserter(added_files, added_files.begin()));

        std::set<std::string> removed_files;
        std::set_difference(old_files.begin(), old_files.end(),
                            current_files_.begin(), current_files_.end(),
                            std::inserter(removed_files, removed_files.begin()));

        auto now = std::chrono::steady_clock::now();
        if (!added_files.empty() || !removed_files.empty()) {
            std::cout << "File system change detected at "
                      << std::chrono::duration_cast<std::chrono::milliseconds>(now - last_check_).count()
                      << "ms interval" << std::endl;

            for (const auto& file : added_files) {
                std::cout << "  Added: " << file << std::endl;
            }
            for (const auto& file : removed_files) {
                std::cout << "  Removed: " << file << std::endl;
            }
        }
        last_check_ = now;
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