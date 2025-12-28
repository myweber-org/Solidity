
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <filesystem>
#include <functional>
#include <atomic>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/inotify.h>
#include <unistd.h>
#include <limits.h>
#endif

class FileSystemWatcher {
public:
    enum class EventType {
        Created,
        Modified,
        Deleted,
        Renamed
    };

    using Callback = std::function<void(const std::filesystem::path&, EventType)>;

    FileSystemWatcher() : running_(false) {}

    ~FileSystemWatcher() {
        stop();
    }

    void addWatchPath(const std::filesystem::path& path) {
        if (std::filesystem::exists(path)) {
            watch_paths_.push_back(path);
        }
    }

    void setCallback(Callback cb) {
        callback_ = cb;
    }

    void start() {
        if (running_) return;
        
        running_ = true;
        watch_thread_ = std::thread(&FileSystemWatcher::watchLoop, this);
    }

    void stop() {
        running_ = false;
        if (watch_thread_.joinable()) {
            watch_thread_.join();
        }
    }

private:
    void watchLoop() {
        #ifdef _WIN32
        watchWindows();
        #else
        watchLinux();
        #endif
    }

    #ifdef _WIN32
    void watchWindows() {
        std::vector<HANDLE> dir_handles;
        
        for (const auto& path : watch_paths_) {
            HANDLE hDir = CreateFileW(
                path.wstring().c_str(),
                FILE_LIST_DIRECTORY,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                NULL,
                OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                NULL
            );

            if (hDir != INVALID_HANDLE_VALUE) {
                dir_handles.push_back(hDir);
            }
        }

        while (running_ && !dir_handles.empty()) {
            std::vector<OVERLAPPED> overlapped(dir_handles.size());
            std::vector<FILE_NOTIFY_INFORMATION> buffer(dir_handles.size() * 1024);

            for (size_t i = 0; i < dir_handles.size(); ++i) {
                ZeroMemory(&overlapped[i], sizeof(OVERLAPPED));
                DWORD bytes_returned;
                
                if (ReadDirectoryChangesW(
                    dir_handles[i],
                    &buffer[i],
                    sizeof(FILE_NOTIFY_INFORMATION) * 1024,
                    TRUE,
                    FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                    FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
                    FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
                    &bytes_returned,
                    &overlapped[i],
                    NULL)) {
                    
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    
                    if (HasOverlappedIoCompleted(&overlapped[i])) {
                        processWindowsEvent(buffer[i], watch_paths_[i]);
                    }
                }
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        for (auto hDir : dir_handles) {
            CloseHandle(hDir);
        }
    }

    void processWindowsEvent(const FILE_NOTIFY_INFORMATION& info, const std::filesystem::path& base_path) {
        std::wstring filename(info.FileName, info.FileNameLength / sizeof(WCHAR));
        std::filesystem::path full_path = base_path / filename;

        EventType event_type;
        switch (info.Action) {
            case FILE_ACTION_ADDED:
            case FILE_ACTION_RENAMED_NEW_NAME:
                event_type = EventType::Created;
                break;
            case FILE_ACTION_MODIFIED:
                event_type = EventType::Modified;
                break;
            case FILE_ACTION_REMOVED:
            case FILE_ACTION_RENAMED_OLD_NAME:
                event_type = EventType::Deleted;
                break;
            default:
                return;
        }

        if (callback_) {
            callback_(full_path, event_type);
        }
    }
    #else
    void watchLinux() {
        int inotify_fd = inotify_init();
        if (inotify_fd < 0) {
            return;
        }

        std::vector<int> watch_descriptors;
        
        for (const auto& path : watch_paths_) {
            int wd = inotify_add_watch(inotify_fd, path.c_str(),
                                      IN_CREATE | IN_MODIFY | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);
            if (wd >= 0) {
                watch_descriptors.push_back(wd);
            }
        }

        char buffer[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
        
        while (running_ && !watch_descriptors.empty()) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(inotify_fd, &fds);

            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            int ready = select(inotify_fd + 1, &fds, NULL, NULL, &timeout);
            
            if (ready > 0 && FD_ISSET(inotify_fd, &fds)) {
                ssize_t len = read(inotify_fd, buffer, sizeof(buffer));
                if (len > 0) {
                    processLinuxEvents(buffer, len);
                }
            }
        }

        for (int wd : watch_descriptors) {
            inotify_rm_watch(inotify_fd, wd);
        }
        close(inotify_fd);
    }

    void processLinuxEvents(const char* buffer, ssize_t length) {
        const struct inotify_event* event;
        
        for (const char* ptr = buffer; ptr < buffer + length; 
             ptr += sizeof(struct inotify_event) + event->len) {
            
            event = reinterpret_cast<const struct inotify_event*>(ptr);
            
            if (event->len > 0) {
                std::filesystem::path file_path(event->name);
                EventType event_type;
                
                if (event->mask & IN_CREATE || event->mask & IN_MOVED_TO) {
                    event_type = EventType::Created;
                } else if (event->mask & IN_MODIFY) {
                    event_type = EventType::Modified;
                } else if (event->mask & IN_DELETE || event->mask & IN_MOVED_FROM) {
                    event_type = EventType::Deleted;
                } else {
                    continue;
                }

                if (callback_) {
                    callback_(file_path, event_type);
                }
            }
        }
    }
    #endif

private:
    std::vector<std::filesystem::path> watch_paths_;
    Callback callback_;
    std::thread watch_thread_;
    std::atomic<bool> running_;
};