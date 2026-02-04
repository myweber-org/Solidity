
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <unordered_map>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const fs::path& watch_path, std::chrono::milliseconds interval)
        : watch_path_(watch_path), interval_(interval), running_(false) {
        if (!fs::exists(watch_path_) || !fs::is_directory(watch_path_)) {
            throw std::runtime_error("Path does not exist or is not a directory");
        }
        snapshot_ = take_snapshot();
    }

    void start() {
        running_ = true;
        watcher_thread_ = std::thread(&FileSystemWatcher::watch_loop, this);
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            running_ = false;
        }
        cv_.notify_all();
        if (watcher_thread_.joinable()) {
            watcher_thread_.join();
        }
    }

    ~FileSystemWatcher() {
        if (running_) {
            stop();
        }
    }

private:
    using FileSnapshot = std::unordered_map<std::string, fs::file_time_type>;

    FileSnapshot take_snapshot() {
        FileSnapshot snapshot;
        for (const auto& entry : fs::recursive_directory_iterator(watch_path_)) {
            if (fs::is_regular_file(entry.status())) {
                snapshot[entry.path().string()] = fs::last_write_time(entry);
            }
        }
        return snapshot;
    }

    void watch_loop() {
        while (running_) {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait_for(lock, interval_, [this] { return !running_; });
            
            if (!running_) break;

            auto new_snapshot = take_snapshot();
            detect_changes(new_snapshot);
            snapshot_ = std::move(new_snapshot);
        }
    }

    void detect_changes(const FileSnapshot& new_snapshot) {
        for (const auto& [path, time] : new_snapshot) {
            auto old_it = snapshot_.find(path);
            if (old_it == snapshot_.end()) {
                std::cout << "File created: " << path << std::endl;
            } else if (old_it->second != time) {
                std::cout << "File modified: " << path << std::endl;
            }
        }

        for (const auto& [path, time] : snapshot_) {
            if (new_snapshot.find(path) == new_snapshot.end()) {
                std::cout << "File deleted: " << path << std::endl;
            }
        }
    }

    fs::path watch_path_;
    std::chrono::milliseconds interval_;
    std::atomic<bool> running_;
    std::thread watcher_thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    FileSnapshot snapshot_;
};

int main() {
    try {
        FileSystemWatcher watcher(".", std::chrono::seconds(2));
        watcher.start();
        
        std::this_thread::sleep_for(std::chrono::seconds(10));
        
        watcher.stop();
        std::cout << "File system watcher stopped." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}#include <iostream>
#include <sys/inotify.h>
#include <unistd.h>
#include <cstring>
#include <limits.h>
#include <errno.h>

class FileSystemWatcher {
private:
    int inotifyFd;
    int watchDescriptor;
    static constexpr size_t EVENT_SIZE = sizeof(struct inotify_event);
    static constexpr size_t BUF_LEN = 1024 * (EVENT_SIZE + NAME_MAX + 1);

public:
    FileSystemWatcher() : inotifyFd(-1), watchDescriptor(-1) {}

    bool initialize() {
        inotifyFd = inotify_init();
        if (inotifyFd < 0) {
            std::cerr << "Error initializing inotify: " << strerror(errno) << std::endl;
            return false;
        }
        return true;
    }

    bool addWatch(const std::string& path, uint32_t mask) {
        if (inotifyFd < 0) {
            std::cerr << "Inotify not initialized" << std::endl;
            return false;
        }

        watchDescriptor = inotify_add_watch(inotifyFd, path.c_str(), mask);
        if (watchDescriptor < 0) {
            std::cerr << "Error adding watch for " << path << ": " << strerror(errno) << std::endl;
            return false;
        }

        std::cout << "Watching: " << path << " (Descriptor: " << watchDescriptor << ")" << std::endl;
        return true;
    }

    void startMonitoring() {
        if (inotifyFd < 0 || watchDescriptor < 0) {
            std::cerr << "Watcher not properly initialized" << std::endl;
            return;
        }

        char buffer[BUF_LEN];
        std::cout << "Starting filesystem monitoring. Press Ctrl+C to stop." << std::endl;

        while (true) {
            ssize_t length = read(inotifyFd, buffer, BUF_LEN);
            if (length < 0) {
                std::cerr << "Read error: " << strerror(errno) << std::endl;
                break;
            }

            ssize_t i = 0;
            while (i < length) {
                struct inotify_event* event = reinterpret_cast<struct inotify_event*>(&buffer[i]);
                if (event->len) {
                    if (event->mask & IN_CREATE) {
                        std::cout << "File created: " << event->name << std::endl;
                    }
                    if (event->mask & IN_DELETE) {
                        std::cout << "File deleted: " << event->name << std::endl;
                    }
                    if (event->mask & IN_MODIFY) {
                        std::cout << "File modified: " << event->name << std::endl;
                    }
                    if (event->mask & IN_MOVED_FROM) {
                        std::cout << "File moved from: " << event->name << std::endl;
                    }
                    if (event->mask & IN_MOVED_TO) {
                        std::cout << "File moved to: " << event->name << std::endl;
                    }
                }
                i += EVENT_SIZE + event->len;
            }
        }
    }

    ~FileSystemWatcher() {
        if (watchDescriptor >= 0 && inotifyFd >= 0) {
            inotify_rm_watch(inotifyFd, watchDescriptor);
        }
        if (inotifyFd >= 0) {
            close(inotifyFd);
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    FileSystemWatcher watcher;
    if (!watcher.initialize()) {
        return 1;
    }

    uint32_t watchMask = IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVE;
    if (!watcher.addWatch(argv[1], watchMask)) {
        return 1;
    }

    watcher.startMonitoring();
    return 0;
}