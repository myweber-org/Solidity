
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
        scheduleNextCheck();
    }

    void stop() {
        running_ = false;
        timer_.cancel();
    }

private:
    void scheduleNextCheck() {
        if (!running_) return;
        timer_.expires_after(std::chrono::seconds(2));
        timer_.async_wait([this](const boost::system::error_code& ec) {
            if (!ec && running_) {
                checkForChanges();
                scheduleNextCheck();
            }
        });
    }

    void checkForChanges() {
        auto current = takeSnapshot();
        auto added = diffSnapshots(snapshot_, current);
        auto removed = diffSnapshots(current, snapshot_);

        for (const auto& file : added) {
            std::cout << "[+] File added: " << file << std::endl;
        }
        for (const auto& file : removed) {
            std::cout << "[-] File removed: " << file << std::endl;
        }

        if (!added.empty() || !removed.empty()) {
            snapshot_ = current;
        }
    }

    std::set<std::string> takeSnapshot() {
        std::set<std::string> snapshot;
        if (!fs::exists(watch_path_)) return snapshot;

        for (const auto& entry : fs::directory_iterator(watch_path_)) {
            if (fs::is_regular_file(entry.path())) {
                snapshot.insert(entry.path().filename().string());
            }
        }
        return snapshot;
    }

    std::set<std::string> diffSnapshots(const std::set<std::string>& oldSet,
                                        const std::set<std::string>& newSet) {
        std::set<std::string> diff;
        std::set_difference(newSet.begin(), newSet.end(),
                            oldSet.begin(), oldSet.end(),
                            std::inserter(diff, diff.begin()));
        return diff;
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