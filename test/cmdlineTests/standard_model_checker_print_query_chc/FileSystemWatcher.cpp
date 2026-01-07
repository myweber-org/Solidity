#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path path_to_watch;
    std::unordered_set<std::string> current_files;

    void populate_file_set() {
        current_files.clear();
        if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                current_files.insert(entry.path().filename().string());
            }
        }
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!fs::exists(path_to_watch)) {
            fs::create_directories(path_to_watch);
        }
        populate_file_set();
        std::cout << "Watching directory: " << fs::absolute(path_to_watch) << std::endl;
    }

    void start_monitoring(int interval_seconds = 2) {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            std::unordered_set<std::string> new_files;
            if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
                for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                    new_files.insert(entry.path().filename().string());
                }
            }

            for (const auto& file : new_files) {
                if (current_files.find(file) == current_files.end()) {
                    std::cout << "[+] File added: " << file << std::endl;
                }
            }

            for (const auto& file : current_files) {
                if (new_files.find(file) == new_files.end()) {
                    std::cout << "[-] File removed: " << file << std::endl;
                }
            }

            current_files = std::move(new_files);
        }
    }
};

int main(int argc, char* argv[]) {
    std::string path = ".";
    if (argc > 1) {
        path = argv[1];
    }

    FileSystemWatcher watcher(path);
    watcher.start_monitoring();

    return 0;
}#include <iostream>
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
        : timer_(io), watch_path_(path), last_check_(fs::last_write_time(path)) {
        start_watching();
    }

private:
    void start_watching() {
        timer_.expires_after(std::chrono::seconds(1));
        timer_.async_wait(boost::bind(&FileSystemWatcher::check_file, this,
                                      asio::placeholders::error));
    }

    void check_file(const boost::system::error_code& ec) {
        if (ec) {
            std::cerr << "Timer error: " << ec.message() << std::endl;
            return;
        }

        try {
            if (!fs::exists(watch_path_)) {
                std::cout << "File deleted: " << watch_path_ << std::endl;
                return;
            }

            auto current_time = fs::last_write_time(watch_path_);
            if (current_time != last_check_) {
                std::cout << "File modified: " << watch_path_ << std::endl;
                last_check_ = current_time;
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }

        start_watching();
    }

    asio::steady_timer timer_;
    std::string watch_path_;
    std::time_t last_check_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }

    try {
        asio::io_context io;
        FileSystemWatcher watcher(io, argv[1]);
        io.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}