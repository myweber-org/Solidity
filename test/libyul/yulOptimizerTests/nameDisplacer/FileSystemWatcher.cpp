
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
    bool running = true;

    std::unordered_set<std::string> get_directory_contents() {
        std::unordered_set<std::string> files;
        if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                files.insert(entry.path().filename().string());
            }
        }
        return files;
    }

    void compare_and_log(const std::unordered_set<std::string>& old_files,
                         const std::unordered_set<std::string>& new_files) {
        for (const auto& file : new_files) {
            if (old_files.find(file) == old_files.end()) {
                std::cout << "[ADDED] " << file << std::endl;
            }
        }
        for (const auto& file : old_files) {
            if (new_files.find(file) == new_files.end()) {
                std::cout << "[REMOVED] " << file << std::endl;
            }
        }
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!fs::exists(path_to_watch)) {
            throw std::runtime_error("Path does not exist: " + path);
        }
        current_files = get_directory_contents();
        std::cout << "Watching directory: " << fs::absolute(path_to_watch) << std::endl;
        std::cout << "Initial file count: " << current_files.size() << std::endl;
    }

    void start_watching(int interval_seconds = 2) {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            auto new_files = get_directory_contents();
            compare_and_log(current_files, new_files);
            current_files = new_files;
        }
    }

    void stop() {
        running = false;
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start_watching(1);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
#include <iostream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind/bind.hpp>
#include <unordered_set>
#include <functional>

namespace fs = boost::filesystem;
namespace asio = boost::asio;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher(asio::io_context& io, const fs::path& dir, Callback cb)
        : io_context_(io), directory_(dir), callback_(cb), timer_(io), scan_interval_(1) {
        if (!fs::exists(directory_) || !fs::is_directory(directory_)) {
            throw std::runtime_error("Invalid directory path");
        }
        last_state_ = capture_directory_state();
        start_watching();
    }

    void set_scan_interval(unsigned int seconds) {
        scan_interval_ = seconds;
    }

private:
    std::unordered_set<std::string> capture_directory_state() {
        std::unordered_set<std::string> state;
        for (const auto& entry : fs::recursive_directory_iterator(directory_)) {
            if (fs::is_regular_file(entry.status())) {
                std::string file_id = entry.path().string() + "|" + std::to_string(fs::last_write_time(entry.path()));
                state.insert(file_id);
            }
        }
        return state;
    }

    void compare_states(const std::unordered_set<std::string>& old_state,
                        const std::unordered_set<std::string>& new_state) {
        for (const auto& item : new_state) {
            if (old_state.find(item) == old_state.end()) {
                size_t sep_pos = item.find('|');
                if (sep_pos != std::string::npos) {
                    std::string file_path = item.substr(0, sep_pos);
                    callback_(file_path, "created_or_modified");
                }
            }
        }
        for (const auto& item : old_state) {
            if (new_state.find(item) == new_state.end()) {
                size_t sep_pos = item.find('|');
                if (sep_pos != std::string::npos) {
                    std::string file_path = item.substr(0, sep_pos);
                    callback_(file_path, "deleted");
                }
            }
        }
    }

    void start_watching() {
        timer_.expires_after(std::chrono::seconds(scan_interval_));
        timer_.async_wait([this](const boost::system::error_code& ec) {
            if (!ec) {
                auto current_state = capture_directory_state();
                compare_states(last_state_, current_state);
                last_state_ = std::move(current_state);
                start_watching();
            }
        });
    }

    asio::io_context& io_context_;
    fs::path directory_;
    Callback callback_;
    asio::steady_timer timer_;
    unsigned int scan_interval_;
    std::unordered_set<std::string> last_state_;
};

void handle_file_event(const fs::path& file_path, const std::string& event_type) {
    std::cout << "File: " << file_path.string() << " Event: " << event_type << std::endl;
}

int main() {
    try {
        asio::io_context io;
        fs::path watch_dir = "./watch_directory";
        fs::create_directories(watch_dir);

        FileSystemWatcher watcher(io, watch_dir, handle_file_event);
        watcher.set_scan_interval(2);

        std::cout << "Watching directory: " << watch_dir.string() << std::endl;
        std::cout << "Press Ctrl+C to stop." << std::endl;

        io.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}