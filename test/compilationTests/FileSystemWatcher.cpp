
#include <iostream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind/bind.hpp>
#include <unordered_set>

namespace fs = boost::filesystem;
namespace asio = boost::asio;

class FileSystemWatcher {
public:
    FileSystemWatcher(asio::io_context& io, const std::string& path)
        : timer_(io), watch_path_(path), running_(false) {
        if (fs::exists(watch_path_) && fs::is_directory(watch_path_)) {
            snapshot_files();
        }
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
    void snapshot_files() {
        previous_files_.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (fs::is_regular_file(entry.path())) {
                previous_files_.insert(fs::canonical(entry.path()).string());
            }
        }
    }

    void schedule_check() {
        timer_.expires_after(std::chrono::seconds(2));
        timer_.async_wait([this](const boost::system::error_code& ec) {
            if (!ec && running_) {
                check_for_changes();
                schedule_check();
            }
        });
    }

    void check_for_changes() {
        std::unordered_set<std::string> current_files;
        std::vector<std::string> added_files, removed_files;

        try {
            for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
                if (fs::is_regular_file(entry.path())) {
                    current_files.insert(fs::canonical(entry.path()).string());
                }
            }

            for (const auto& file : current_files) {
                if (previous_files_.find(file) == previous_files_.end()) {
                    added_files.push_back(file);
                }
            }

            for (const auto& file : previous_files_) {
                if (current_files.find(file) == current_files.end()) {
                    removed_files.push_back(file);
                }
            }

            if (!added_files.empty() || !removed_files.empty()) {
                std::cout << "File system change detected at " << watch_path_ << std::endl;
                for (const auto& added : added_files) {
                    std::cout << "  [+] " << fs::path(added).filename().string() << std::endl;
                }
                for (const auto& removed : removed_files) {
                    std::cout << "  [-] " << fs::path(removed).filename().string() << std::endl;
                }
                previous_files_.swap(current_files);
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
    }

    asio::steady_timer timer_;
    std::string watch_path_;
    std::unordered_set<std::string> previous_files_;
    bool running_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
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