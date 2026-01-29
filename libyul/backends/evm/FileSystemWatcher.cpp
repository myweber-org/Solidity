
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
}#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const fs::path& directory) : watch_directory(directory) {
        if (!fs::exists(watch_directory) || !fs::is_directory(watch_directory)) {
            throw std::runtime_error("Provided path is not a valid directory.");
        }
        populate_file_map();
    }

    void start_monitoring(int interval_seconds = 1) {
        std::cout << "Starting to monitor: " << watch_directory << std::endl;
        while (monitoring_active) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

    void stop_monitoring() {
        monitoring_active = false;
    }

private:
    fs::path watch_directory;
    std::unordered_map<std::string, fs::file_time_type> file_map;
    bool monitoring_active{true};

    void populate_file_map() {
        file_map.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.path())) {
                file_map[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void check_for_changes() {
        auto current_map = file_map;
        bool changes_detected = false;

        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (!fs::is_regular_file(entry.path())) continue;

            std::string file_path = entry.path().string();
            auto current_write_time = fs::last_write_time(entry.path());

            if (current_map.find(file_path) == current_map.end()) {
                std::cout << "[NEW] File created: " << file_path << std::endl;
                changes_detected = true;
            } else if (current_map[file_path] != current_write_time) {
                std::cout << "[MODIFIED] File changed: " << file_path << std::endl;
                changes_detected = true;
            }
            current_map.erase(file_path);
        }

        for (const auto& [missing_file, _] : current_map) {
            std::cout << "[DELETED] File removed: " << missing_file << std::endl;
            changes_detected = true;
        }

        if (changes_detected) {
            populate_file_map();
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
        watcher.start_monitoring(2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}