
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
            snapshot_ = getDirectorySnapshot();
        }
    }

    void start() {
        if (running_) return;
        running_ = true;
        scheduleNextCheck();
    }

    void stop() {
        running_ = false;
        timer_.cancel();
    }

private:
    using FileSet = std::set<std::string>;

    FileSet getDirectorySnapshot() {
        FileSet files;
        if (!fs::exists(watch_path_)) return files;

        fs::directory_iterator end;
        for (fs::directory_iterator it(watch_path_); it != end; ++it) {
            if (fs::is_regular_file(*it)) {
                files.insert(it->path().filename().string());
            }
        }
        return files;
    }

    void checkForChanges() {
        if (!running_) return;

        FileSet current = getDirectorySnapshot();
        FileSet added, removed;

        std::set_difference(current.begin(), current.end(),
                           snapshot_.begin(), snapshot_.end(),
                           std::inserter(added, added.begin()));

        std::set_difference(snapshot_.begin(), snapshot_.end(),
                           current.begin(), current.end(),
                           std::inserter(removed, removed.begin()));

        if (!added.empty() || !removed.empty()) {
            handleChanges(added, removed);
            snapshot_ = current;
        }

        scheduleNextCheck();
    }

    void handleChanges(const FileSet& added, const FileSet& removed) {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);

        if (!added.empty()) {
            std::cout << "[" << std::ctime(&time) << "] Files added: ";
            for (const auto& file : added) {
                std::cout << file << " ";
            }
            std::cout << std::endl;
        }

        if (!removed.empty()) {
            std::cout << "[" << std::ctime(&time) << "] Files removed: ";
            for (const auto& file : removed) {
                std::cout << file << " ";
            }
            std::cout << std::endl;
        }
    }

    void scheduleNextCheck() {
        timer_.expires_after(std::chrono::seconds(1));
        timer_.async_wait([this](const boost::system::error_code& ec) {
            if (!ec && running_) {
                checkForChanges();
            }
        });
    }

    asio::steady_timer timer_;
    std::string watch_path_;
    FileSet snapshot_;
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

        std::cout << "Watching directory: " << argv[1] << std::endl;
        std::cout << "Press Ctrl+C to stop..." << std::endl;

        watcher.start();

        asio::signal_set signals(io, SIGINT, SIGTERM);
        signals.async_wait([&watcher, &io](const boost::system::error_code&, int) {
            watcher.stop();
            io.stop();
        });

        io.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}