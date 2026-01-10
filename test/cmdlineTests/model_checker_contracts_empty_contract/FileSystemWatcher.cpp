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
}#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>
#include <functional>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/inotify.h>
    #include <unistd.h>
    #include <limits.h>
#endif

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher(const fs::path& path, Callback callback)
        : watch_path_(fs::absolute(path)), callback_(callback), running_(false) {
        if (!fs::exists(watch_path_) || !fs::is_directory(watch_path_)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

    void start() {
        running_ = true;
        worker_thread_ = std::thread(&FileSystemWatcher::watch_loop, this);
    }

    void stop() {
        running_ = false;
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
    }

private:
    fs::path watch_path_;
    Callback callback_;
    bool running_;
    std::thread worker_thread_;
    std::unordered_set<std::string> known_files_;

    void watch_loop() {
        scan_initial_files();

#ifdef _WIN32
        watch_loop_windows();
#else
        watch_loop_linux();
#endif
    }

    void scan_initial_files() {
        known_files_.clear();
        for (const auto& entry : fs::directory_iterator(watch_path_)) {
            if (entry.is_regular_file()) {
                known_files_.insert(entry.path().filename().string());
            }
        }
    }

    void detect_changes() {
        std::unordered_set<std::string> current_files;

        for (const auto& entry : fs::directory_iterator(watch_path_)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                current_files.insert(filename);

                if (known_files_.find(filename) == known_files_.end()) {
                    callback_(entry.path(), "created");
                }
            }
        }

        for (const auto& old_file : known_files_) {
            if (current_files.find(old_file) == current_files.end()) {
                callback_(watch_path_ / old_file, "deleted");
            }
        }

        known_files_.swap(current_files);
    }

#ifdef _WIN32
    void watch_loop_windows() {
        HANDLE dir_handle = CreateFileW(
            watch_path_.c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,
            nullptr
        );

        if (dir_handle == INVALID_HANDLE_VALUE) return;

        constexpr DWORD buffer_size = 4096;
        std::vector<BYTE> buffer(buffer_size);

        while (running_) {
            DWORD bytes_returned = 0;
            if (ReadDirectoryChangesW(
                dir_handle,
                buffer.data(),
                buffer_size,
                TRUE,
                FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
                &bytes_returned,
                nullptr,
                nullptr)) {

                if (bytes_returned > 0) {
                    detect_changes();
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        CloseHandle(dir_handle);
    }
#else
    void watch_loop_linux() {
        int inotify_fd = inotify_init();
        if (inotify_fd < 0) return;

        int watch_desc = inotify_add_watch(inotify_fd, watch_path_.c_str(),
                                          IN_CREATE | IN_DELETE | IN_MODIFY);
        if (watch_desc < 0) {
            close(inotify_fd);
            return;
        }

        constexpr size_t event_size = sizeof(struct inotify_event);
        constexpr size_t buffer_size = 1024 * (event_size + NAME_MAX + 1);
        std::vector<char> buffer(buffer_size);

        while (running_) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(inotify_fd, &fds);

            struct timeval timeout = {1, 0};

            int select_result = select(inotify_fd + 1, &fds, nullptr, nullptr, &timeout);
            if (select_result > 0 && FD_ISSET(inotify_fd, &fds)) {
                ssize_t length = read(inotify_fd, buffer.data(), buffer_size);
                if (length > 0) {
                    detect_changes();
                }
            } else if (select_result < 0) {
                break;
            }
        }

        inotify_rm_watch(inotify_fd, watch_desc);
        close(inotify_fd);
    }
#endif
};

void example_callback(const fs::path& path, const std::string& action) {
    std::cout << "File " << path.filename() << " was " << action << " at "
              << std::chrono::system_clock::now().time_since_epoch().count()
              << " nanoseconds since epoch.\n";
}

int main() {
    try {
        fs::path current_dir = fs::current_path();
        FileSystemWatcher watcher(current_dir, example_callback);

        std::cout << "Watching directory: " << current_dir << "\n";
        std::cout << "Press Enter to stop watching...\n";

        watcher.start();

        std::cin.get();
        watcher.stop();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}