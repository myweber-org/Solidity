#include <iostream>
#include <chrono>
#include <thread>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind/bind.hpp>
#include <boost/system/error_code.hpp>
#include <set>

namespace fs = boost::filesystem;
namespace asio = boost::asio;

class FileSystemWatcher {
public:
    FileSystemWatcher(asio::io_context& io_context, const std::string& path)
        : io_context_(io_context),
          path_(path),
          timer_(io_context),
          scan_interval_(1) {
        last_state_ = getCurrentState();
    }

    void start() {
        scheduleScan();
    }

    void setScanInterval(int seconds) {
        scan_interval_ = seconds;
    }

    void onFileChanged(std::function<void(const std::string&)> callback) {
        file_changed_callback_ = callback;
    }

    void onFileRemoved(std::function<void(const std::string&)> callback) {
        file_removed_callback_ = callback;
    }

    void onFileAdded(std::function<void(const std::string&)> callback) {
        file_added_callback_ = callback;
    }

private:
    std::set<std::string> getCurrentState() {
        std::set<std::string> current_files;
        if (fs::exists(path_) && fs::is_directory(path_)) {
            for (const auto& entry : fs::directory_iterator(path_)) {
                if (fs::is_regular_file(entry.path())) {
                    current_files.insert(entry.path().string());
                }
            }
        }
        return current_files;
    }

    void compareStates(const std::set<std::string>& old_state,
                       const std::set<std::string>& new_state) {
        std::set<std::string> added_files;
        std::set<std::string> removed_files;

        std::set_difference(new_state.begin(), new_state.end(),
                           old_state.begin(), old_state.end(),
                           std::inserter(added_files, added_files.begin()));

        std::set_difference(old_state.begin(), old_state.end(),
                           new_state.begin(), new_state.end(),
                           std::inserter(removed_files, removed_files.begin()));

        for (const auto& file : added_files) {
            if (file_added_callback_) {
                file_added_callback_(file);
            }
        }

        for (const auto& file : removed_files) {
            if (file_removed_callback_) {
                file_removed_callback_(file);
            }
        }

        std::set<std::string> common_files;
        std::set_intersection(old_state.begin(), old_state.end(),
                             new_state.begin(), new_state.end(),
                             std::inserter(common_files, common_files.begin()));

        for (const auto& file : common_files) {
            if (hasFileChanged(file)) {
                if (file_changed_callback_) {
                    file_changed_callback_(file);
                }
            }
        }
    }

    bool hasFileChanged(const std::string& filepath) {
        static std::map<std::string, std::time_t> last_mod_times;

        try {
            auto current_mod_time = fs::last_write_time(filepath);
            if (last_mod_times.find(filepath) == last_mod_times.end()) {
                last_mod_times[filepath] = current_mod_time;
                return false;
            }

            if (last_mod_times[filepath] != current_mod_time) {
                last_mod_times[filepath] = current_mod_time;
                return true;
            }
        } catch (const fs::filesystem_error&) {
            return false;
        }

        return false;
    }

    void scheduleScan() {
        timer_.expires_after(std::chrono::seconds(scan_interval_));
        timer_.async_wait(boost::bind(&FileSystemWatcher::performScan, this,
                                     asio::placeholders::error));
    }

    void performScan(const boost::system::error_code& ec) {
        if (ec) {
            return;
        }

        auto current_state = getCurrentState();
        compareStates(last_state_, current_state);
        last_state_ = current_state;

        scheduleScan();
    }

    asio::io_context& io_context_;
    std::string path_;
    asio::steady_timer timer_;
    int scan_interval_;
    std::set<std::string> last_state_;

    std::function<void(const std::string&)> file_changed_callback_;
    std::function<void(const std::string&)> file_removed_callback_;
    std::function<void(const std::string&)> file_added_callback_;
};

int main() {
    try {
        asio::io_context io_context;
        FileSystemWatcher watcher(io_context, ".");

        watcher.onFileAdded([](const std::string& file) {
            std::cout << "File added: " << file << std::endl;
        });

        watcher.onFileRemoved([](const std::string& file) {
            std::cout << "File removed: " << file << std::endl;
        });

        watcher.onFileChanged([](const std::string& file) {
            std::cout << "File changed: " << file << std::endl;
        });

        watcher.start();
        io_context.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}