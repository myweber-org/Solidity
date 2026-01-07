
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
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

    FileSystemWatcher() : running_(false) {}

    ~FileSystemWatcher() {
        stop();
    }

    void addWatch(const fs::path& directory, Callback callback) {
        watch_map_[directory] = callback;
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
        #ifdef _WIN32
        monitorWindows();
        #else
        monitorLinux();
        #endif
    }

    #ifdef _WIN32
    void monitorWindows() {
        std::vector<HANDLE> dir_handles;
        std::unordered_map<HANDLE, fs::path> handle_to_path;

        for (const auto& [path, callback] : watch_map_) {
            HANDLE dir_handle = CreateFile(
                path.c_str(),
                FILE_LIST_DIRECTORY,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                nullptr,
                OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                nullptr
            );

            if (dir_handle != INVALID_HANDLE_VALUE) {
                dir_handles.push_back(dir_handle);
                handle_to_path[dir_handle] = path;
            }
        }

        while (running_ && !dir_handles.empty()) {
            DWORD wait_result = WaitForMultipleObjects(
                dir_handles.size(), dir_handles.data(), FALSE, 500
            );

            if (wait_result >= WAIT_OBJECT_0 && wait_result < WAIT_OBJECT_0 + dir_handles.size()) {
                HANDLE changed_handle = dir_handles[wait_result - WAIT_OBJECT_0];
                fs::path watch_path = handle_to_path[changed_handle];

                BYTE buffer[4096];
                DWORD bytes_returned;
                if (ReadDirectoryChangesW(
                    changed_handle,
                    buffer,
                    sizeof(buffer),
                    TRUE,
                    FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                    FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
                    FILE_NOTIFY_CHANGE_LAST_WRITE,
                    &bytes_returned,
                    nullptr,
                    nullptr
                )) {
                    FILE_NOTIFY_INFORMATION* notify_info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
                    do {
                        std::wstring filename(notify_info->FileName, notify_info->FileNameLength / sizeof(WCHAR));
                        fs::path full_path = watch_path / filename;
                        
                        std::string action;
                        switch (notify_info->Action) {
                            case FILE_ACTION_ADDED: action = "ADDED"; break;
                            case FILE_ACTION_REMOVED: action = "REMOVED"; break;
                            case FILE_ACTION_MODIFIED: action = "MODIFIED"; break;
                            case FILE_ACTION_RENAMED_OLD_NAME: action = "RENAMED_OLD"; break;
                            case FILE_ACTION_RENAMED_NEW_NAME: action = "RENAMED_NEW"; break;
                            default: action = "UNKNOWN"; break;
                        }

                        if (watch_map_.count(watch_path)) {
                            watch_map_[watch_path](full_path, action);
                        }

                        if (notify_info->NextEntryOffset == 0) break;
                        notify_info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                            reinterpret_cast<BYTE*>(notify_info) + notify_info->NextEntryOffset
                        );
                    } while (true);
                }
            }
        }

        for (HANDLE handle : dir_handles) {
            CloseHandle(handle);
        }
    }
    #else
    void monitorLinux() {
        int inotify_fd = inotify_init1(IN_NONBLOCK);
        if (inotify_fd < 0) return;

        std::unordered_map<int, fs::path> wd_to_path;

        for (const auto& [path, callback] : watch_map_) {
            int wd = inotify_add_watch(inotify_fd, path.c_str(),
                IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO);
            if (wd >= 0) {
                wd_to_path[wd] = path;
            }
        }

        char buffer[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
        const struct inotify_event* event;

        while (running_) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(inotify_fd, &fds);

            struct timeval timeout = {0, 500000};
            int ready = select(inotify_fd + 1, &fds, nullptr, nullptr, &timeout);

            if (ready > 0 && FD_ISSET(inotify_fd, &fds)) {
                ssize_t len = read(inotify_fd, buffer, sizeof(buffer));
                if (len <= 0) continue;

                for (char* ptr = buffer; ptr < buffer + len; 
                     ptr += sizeof(struct inotify_event) + event->len) {
                    event = reinterpret_cast<const struct inotify_event*>(ptr);
                    
                    if (wd_to_path.count(event->wd)) {
                        fs::path watch_path = wd_to_path[event->wd];
                        fs::path full_path = watch_path / event->name;
                        
                        std::string action;
                        if (event->mask & IN_CREATE) action = "CREATED";
                        else if (event->mask & IN_DELETE) action = "DELETED";
                        else if (event->mask & IN_MODIFY) action = "MODIFIED";
                        else if (event->mask & IN_MOVED_FROM) action = "MOVED_FROM";
                        else if (event->mask & IN_MOVED_TO) action = "MOVED_TO";
                        else action = "UNKNOWN";

                        if (watch_map_.count(watch_path)) {
                            watch_map_[watch_path](full_path, action);
                        }
                    }
                }
            }
        }

        close(inotify_fd);
    }
    #endif

    std::unordered_map<fs::path, Callback> watch_map_;
    std::thread monitor_thread_;
    std::atomic<bool> running_;
};

int main() {
    FileSystemWatcher watcher;
    
    watcher.addWatch(".", [](const fs::path& path, const std::string& action) {
        std::cout << "File " << path << " was " << action << std::endl;
    });
    
    watcher.addWatch("/tmp", [](const fs::path& path, const std::string& action) {
        std::cout << "Temp file " << path << " was " << action << std::endl;
    });
    
    watcher.start();
    
    std::cout << "Watching for file system changes. Press Enter to stop..." << std::endl;
    std::cin.get();
    
    watcher.stop();
    return 0;
}