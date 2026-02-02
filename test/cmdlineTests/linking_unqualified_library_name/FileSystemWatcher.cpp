#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
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
    enum class EventType {
        Created,
        Modified,
        Deleted,
        Renamed
    };

    struct FileEvent {
        std::filesystem::path path;
        EventType type;
        std::chrono::system_clock::time_point timestamp;
    };

    FileSystemWatcher(const std::filesystem::path& watch_path, bool recursive = false)
        : watch_path_(watch_path), recursive_(recursive), running_(false) {
        if (!std::filesystem::exists(watch_path_)) {
            throw std::runtime_error("Watch path does not exist");
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

    void start() {
        if (running_) return;
        
        running_ = true;
        watch_thread_ = std::thread(&FileSystemWatcher::watch_loop, this);
    }

    void stop() {
        running_ = false;
        if (watch_thread_.joinable()) {
            watch_thread_.join();
        }
    }

    std::vector<FileEvent> get_events() {
        std::lock_guard<std::mutex> lock(events_mutex_);
        std::vector<FileEvent> events = events_;
        events_.clear();
        return events;
    }

private:
    void watch_loop() {
#ifdef _WIN32
        watch_windows();
#else
        watch_linux();
#endif
    }

#ifdef _WIN32
    void watch_windows() {
        HANDLE dir_handle = CreateFileW(
            watch_path_.wstring().c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            NULL
        );

        if (dir_handle == INVALID_HANDLE_VALUE) {
            return;
        }

        char buffer[4096];
        DWORD bytes_returned;
        OVERLAPPED overlapped = {0};
        overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        while (running_) {
            if (ReadDirectoryChangesW(
                dir_handle,
                buffer,
                sizeof(buffer),
                recursive_,
                FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
                FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
                &bytes_returned,
                &overlapped,
                NULL
            )) {
                WaitForSingleObject(overlapped.hEvent, INFINITE);
                
                if (!running_) break;

                FILE_NOTIFY_INFORMATION* notify_info = (FILE_NOTIFY_INFORMATION*)buffer;
                while (notify_info != NULL) {
                    process_windows_event(notify_info);
                    
                    if (notify_info->NextEntryOffset == 0) {
                        break;
                    }
                    notify_info = (FILE_NOTIFY_INFORMATION*)((BYTE*)notify_info + notify_info->NextEntryOffset);
                }
            }
        }

        CloseHandle(overlapped.hEvent);
        CloseHandle(dir_handle);
    }

    void process_windows_event(FILE_NOTIFY_INFORMATION* notify_info) {
        std::wstring filename(notify_info->FileName, notify_info->FileNameLength / sizeof(WCHAR));
        std::filesystem::path full_path = watch_path_ / filename;
        
        EventType type = EventType::Modified;
        switch (notify_info->Action) {
            case FILE_ACTION_ADDED:
                type = EventType::Created;
                break;
            case FILE_ACTION_REMOVED:
                type = EventType::Deleted;
                break;
            case FILE_ACTION_MODIFIED:
                type = EventType::Modified;
                break;
            case FILE_ACTION_RENAMED_OLD_NAME:
            case FILE_ACTION_RENAMED_NEW_NAME:
                type = EventType::Renamed;
                break;
        }

        add_event({full_path, type, std::chrono::system_clock::now()});
    }
#else
    void watch_linux() {
        int inotify_fd = inotify_init();
        if (inotify_fd < 0) {
            return;
        }

        int watch_desc = inotify_add_watch(
            inotify_fd,
            watch_path_.c_str(),
            IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVE | IN_ATTRIB
        );

        if (watch_desc < 0) {
            close(inotify_fd);
            return;
        }

        char buffer[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
        
        while (running_) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(inotify_fd, &fds);

            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            int ready = select(inotify_fd + 1, &fds, NULL, NULL, &timeout);
            
            if (ready > 0 && FD_ISSET(inotify_fd, &fds)) {
                ssize_t len = read(inotify_fd, buffer, sizeof(buffer));
                if (len <= 0) continue;

                char* ptr = buffer;
                while (ptr < buffer + len) {
                    struct inotify_event* event = (struct inotify_event*)ptr;
                    process_linux_event(event);
                    ptr += sizeof(struct inotify_event) + event->len;
                }
            }
        }

        inotify_rm_watch(inotify_fd, watch_desc);
        close(inotify_fd);
    }

    void process_linux_event(struct inotify_event* event) {
        if (event->len == 0) return;
        
        std::filesystem::path full_path = watch_path_ / event->name;
        EventType type = EventType::Modified;
        
        if (event->mask & IN_CREATE) {
            type = EventType::Created;
        } else if (event->mask & IN_DELETE) {
            type = EventType::Deleted;
        } else if (event->mask & IN_MODIFY) {
            type = EventType::Modified;
        } else if (event->mask & (IN_MOVED_FROM | IN_MOVED_TO)) {
            type = EventType::Renamed;
        }

        add_event({full_path, type, std::chrono::system_clock::now()});
    }
#endif

    void add_event(const FileEvent& event) {
        std::lock_guard<std::mutex> lock(events_mutex_);
        events_.push_back(event);
    }

    std::filesystem::path watch_path_;
    bool recursive_;
    std::atomic<bool> running_;
    std::thread watch_thread_;
    std::vector<FileEvent> events_;
    std::mutex events_mutex_;
};