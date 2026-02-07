
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path watch_path;
    std::unordered_set<std::string> known_files;

    void scan_directory() {
        std::unordered_set<std::string> current_files;
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            current_files.insert(entry.path().filename().string());
        }

        for (const auto& file : current_files) {
            if (known_files.find(file) == known_files.end()) {
                std::cout << "[+] File added: " << file << std::endl;
            }
        }

        for (const auto& file : known_files) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "[-] File removed: " << file << std::endl;
            }
        }

        known_files = std::move(current_files);
    }

public:
    explicit FileSystemWatcher(const std::string& path) : watch_path(path) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Invalid directory path");
        }
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            known_files.insert(entry.path().filename().string());
        }
        std::cout << "Watching directory: " << watch_path << std::endl;
    }

    void start_watching(int interval_seconds = 2) {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            try {
                scan_directory();
            } catch (const std::exception& e) {
                std::cerr << "Error scanning directory: " << e.what() << std::endl;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start_watching();
    } catch (const std::exception& e) {
        std::cerr << "Failed to start watcher: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <mutex>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher(const fs::path& watch_path, Callback callback)
        : watch_directory_(watch_path), callback_(callback), running_(false) {
        
        if (!fs::exists(watch_directory_) || !fs::is_directory(watch_directory_)) {
            throw std::runtime_error("Invalid watch directory");
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

    void start() {
        if (running_) return;
        
        running_ = true;
        monitor_thread_ = std::thread(&FileSystemWatcher::monitor, this);
    }

    void stop() {
        running_ = false;
        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
    }

private:
    void monitor() {
        std::unordered_map<std::string, fs::file_time_type> file_timestamps;
        
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory_)) {
            if (fs::is_regular_file(entry.path())) {
                file_timestamps[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }

        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            std::unordered_map<std::string, fs::file_time_type> current_timestamps;
            
            for (const auto& entry : fs::recursive_directory_iterator(watch_directory_)) {
                if (!fs::is_regular_file(entry.path())) continue;
                
                std::string file_path = entry.path().string();
                auto current_time = fs::last_write_time(entry.path());
                current_timestamps[file_path] = current_time;

                auto it = file_timestamps.find(file_path);
                if (it == file_timestamps.end()) {
                    notify(entry.path(), "created");
                } else if (it->second != current_time) {
                    notify(entry.path(), "modified");
                }
            }

            for (const auto& [file_path, _] : file_timestamps) {
                if (current_timestamps.find(file_path) == current_timestamps.end()) {
                    notify(file_path, "deleted");
                }
            }

            file_timestamps = std::move(current_timestamps);
        }
    }

    void notify(const fs::path& file_path, const std::string& event_type) {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        if (callback_) {
            callback_(file_path, event_type);
        }
    }

    fs::path watch_directory_;
    Callback callback_;
    std::atomic<bool> running_;
    std::thread monitor_thread_;
    std::mutex callback_mutex_;
};

void example_callback(const fs::path& path, const std::string& event) {
    std::cout << "File: " << path.filename() << " Event: " << event 
              << " at " << std::chrono::system_clock::now().time_since_epoch().count() 
              << std::endl;
}

int main() {
    try {
        FileSystemWatcher watcher("./test_directory", example_callback);
        watcher.start();
        
        std::cout << "Watching directory. Press Enter to stop..." << std::endl;
        std::cin.get();
        
        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/inotify.h>
#include <unistd.h>
#include <limits.h>
#endif

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const std::string& path) : watch_path(path), running(false) {
        if (!std::filesystem::exists(path)) {
            throw std::runtime_error("Path does not exist: " + path);
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

    void start() {
        running = true;
        watcher_thread = std::thread(&FileSystemWatcher::watch_loop, this);
    }

    void stop() {
        running = false;
        if (watcher_thread.joinable()) {
            watcher_thread.join();
        }
    }

    void set_callback(std::function<void(const std::string&, const std::string&)> cb) {
        callback = cb;
    }

private:
    std::string watch_path;
    std::atomic<bool> running;
    std::thread watcher_thread;
    std::function<void(const std::string&, const std::string&)> callback;

    void watch_loop() {
#ifdef _WIN32
        HANDLE dir_handle = CreateFileA(
            watch_path.c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            nullptr
        );

        if (dir_handle == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to open directory: " << watch_path << std::endl;
            return;
        }

        char buffer[4096];
        DWORD bytes_returned;
        OVERLAPPED overlapped = {0};
        overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

        while (running) {
            if (ReadDirectoryChangesW(
                dir_handle,
                buffer,
                sizeof(buffer),
                TRUE,
                FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
                FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
                &bytes_returned,
                &overlapped,
                nullptr
            )) {
                WaitForSingleObject(overlapped.hEvent, INFINITE);
                if (bytes_returned > 0) {
                    FILE_NOTIFY_INFORMATION* notify_info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
                    do {
                        std::wstring wfilename(notify_info->FileName, notify_info->FileNameLength / sizeof(WCHAR));
                        std::string filename(wfilename.begin(), wfilename.end());
                        std::string action;

                        switch (notify_info->Action) {
                            case FILE_ACTION_ADDED: action = "ADDED"; break;
                            case FILE_ACTION_REMOVED: action = "REMOVED"; break;
                            case FILE_ACTION_MODIFIED: action = "MODIFIED"; break;
                            case FILE_ACTION_RENAMED_OLD_NAME: action = "RENAMED_OLD"; break;
                            case FILE_ACTION_RENAMED_NEW_NAME: action = "RENAMED_NEW"; break;
                            default: action = "UNKNOWN";
                        }

                        if (callback) {
                            callback(filename, action);
                        }

                        if (notify_info->NextEntryOffset == 0) break;
                        notify_info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                            reinterpret_cast<BYTE*>(notify_info) + notify_info->NextEntryOffset
                        );
                    } while (true);
                }
                ResetEvent(overlapped.hEvent);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        CloseHandle(overlapped.hEvent);
        CloseHandle(dir_handle);
#else
        int inotify_fd = inotify_init();
        if (inotify_fd < 0) {
            std::cerr << "Failed to initialize inotify" << std::endl;
            return;
        }

        int watch_desc = inotify_add_watch(inotify_fd, watch_path.c_str(),
                                          IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO);
        if (watch_desc < 0) {
            std::cerr << "Failed to add watch for: " << watch_path << std::endl;
            close(inotify_fd);
            return;
        }

        char buffer[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
        const struct inotify_event* event;

        while (running) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(inotify_fd, &fds);

            struct timeval timeout = {1, 0};
            int ret = select(inotify_fd + 1, &fds, nullptr, nullptr, &timeout);

            if (ret > 0 && FD_ISSET(inotify_fd, &fds)) {
                ssize_t len = read(inotify_fd, buffer, sizeof(buffer));
                if (len <= 0) continue;

                for (char* ptr = buffer; ptr < buffer + len; ptr += sizeof(struct inotify_event) + event->len) {
                    event = reinterpret_cast<const struct inotify_event*>(ptr);
                    std::string filename = event->len > 0 ? event->name : "";
                    std::string action;

                    if (event->mask & IN_CREATE) action = "CREATED";
                    else if (event->mask & IN_DELETE) action = "DELETED";
                    else if (event->mask & IN_MODIFY) action = "MODIFIED";
                    else if (event->mask & IN_MOVED_FROM) action = "MOVED_FROM";
                    else if (event->mask & IN_MOVED_TO) action = "MOVED_TO";
                    else action = "UNKNOWN";

                    if (callback) {
                        callback(filename, action);
                    }
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        inotify_rm_watch(inotify_fd, watch_desc);
        close(inotify_fd);
#endif
    }
};