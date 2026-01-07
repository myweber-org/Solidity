
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
            snapshot_ = takeSnapshot();
        }
    }

    void start() {
        if (running_) return;
        running_ = true;
        scheduleCheck();
    }

    void stop() {
        running_ = false;
        timer_.cancel();
    }

private:
    using FileSet = std::unordered_set<std::string>;

    FileSet takeSnapshot() {
        FileSet files;
        if (!fs::exists(watch_path_)) return files;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (fs::is_regular_file(entry.status())) {
                files.insert(fs::canonical(entry.path()).string());
            }
        }
        return files;
    }

    void scheduleCheck() {
        if (!running_) return;
        timer_.expires_after(std::chrono::seconds(2));
        timer_.async_wait([this](const boost::system::error_code& ec) {
            if (!ec && running_) {
                checkForChanges();
                scheduleCheck();
            }
        });
    }

    void checkForChanges() {
        auto current = takeSnapshot();
        std::vector<std::string> added, removed;

        for (const auto& file : current) {
            if (snapshot_.find(file) == snapshot_.end()) {
                added.push_back(file);
            }
        }

        for (const auto& file : snapshot_) {
            if (current.find(file) == current.end()) {
                removed.push_back(file);
            }
        }

        if (!added.empty() || !removed.empty()) {
            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto& file : added) {
                std::cout << "[ADDED] " << file << std::endl;
            }
            for (const auto& file : removed) {
                std::cout << "[REMOVED] " << file << std::endl;
            }
            snapshot_ = std::move(current);
        }
    }

    asio::steady_timer timer_;
    std::string watch_path_;
    FileSet snapshot_;
    bool running_;
    std::mutex mutex_;
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
}#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& directory) : watch_path(directory) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        populate_file_map();
    }

    void start_monitoring(int interval_seconds = 2) {
        std::cout << "Starting to monitor: " << watch_path.string() << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps;

    void populate_file_map() {
        file_timestamps.clear();
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_timestamps[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void check_for_changes() {
        auto it = file_timestamps.begin();
        while (it != file_timestamps.end()) {
            if (!fs::exists(it->first)) {
                std::cout << "File removed: " << it->first << std::endl;
                it = file_timestamps.erase(it);
            } else {
                ++it;
            }
        }

        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                std::string file_path = entry.path().string();
                auto current_write_time = fs::last_write_time(entry.path());

                if (file_timestamps.find(file_path) == file_timestamps.end()) {
                    std::cout << "New file created: " << file_path << std::endl;
                    file_timestamps[file_path] = current_write_time;
                } else if (file_timestamps[file_path] != current_write_time) {
                    std::cout << "File modified: " << file_path << std::endl;
                    file_timestamps[file_path] = current_write_time;
                }
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start_monitoring();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}