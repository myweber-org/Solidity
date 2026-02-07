
#include <iostream>
#include <chrono>
#include <thread>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind/bind.hpp>
#include <set>

namespace fs = boost::filesystem;
namespace asio = boost::asio;

class FileSystemWatcher {
public:
    FileSystemWatcher(asio::io_context& io, const std::string& path)
        : timer_(io), watch_path_(path), running_(false) {
        if (fs::exists(watch_path_) && fs::is_directory(watch_path_)) {
            snapshot_ = takeSnapshot();
        }
    }

    void start() {
        running_ = true;
        scheduleCheck();
    }

    void stop() {
        running_ = false;
        timer_.cancel();
    }

private:
    void scheduleCheck() {
        if (!running_) return;
        timer_.expires_after(std::chrono::seconds(1));
        timer_.async_wait([this](const boost::system::error_code& ec) {
            if (!ec) {
                checkForChanges();
                scheduleCheck();
            }
        });
    }

    std::set<std::string> takeSnapshot() {
        std::set<std::string> snapshot;
        if (fs::exists(watch_path_) && fs::is_directory(watch_path_)) {
            for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
                if (fs::is_regular_file(entry.path())) {
                    snapshot.insert(fs::canonical(entry.path()).string());
                }
            }
        }
        return snapshot;
    }

    void checkForChanges() {
        auto current = takeSnapshot();
        std::set<std::string> added, removed;

        std::set_difference(current.begin(), current.end(),
                           snapshot_.begin(), snapshot_.end(),
                           std::inserter(added, added.begin()));

        std::set_difference(snapshot_.begin(), snapshot_.end(),
                           current.begin(), current.end(),
                           std::inserter(removed, removed.begin()));

        if (!added.empty() || !removed.empty()) {
            std::cout << "File system changes detected in: " << watch_path_ << std::endl;
            for (const auto& file : added) {
                std::cout << "  [+] " << file << std::endl;
            }
            for (const auto& file : removed) {
                std::cout << "  [-] " << file << std::endl;
            }
            snapshot_ = current;
        }
    }

    asio::steady_timer timer_;
    std::string watch_path_;
    std::set<std::string> snapshot_;
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