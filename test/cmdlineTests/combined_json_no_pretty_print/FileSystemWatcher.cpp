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

    FileSystemWatcher(const std::filesystem::path& watch_path) 
        : watch_path_(watch_path), running_(false) {
        if (!std::filesystem::exists(watch_path)) {
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
        auto events = events_;
        events_.clear();
        return events;
    }

private:
    void watch_loop() {
#ifdef _WIN32
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

        std::vector<char> buffer(65536);
        OVERLAPPED overlapped = {0};
        overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        while (running_) {
            DWORD bytes_returned = 0;
            ResetEvent(overlapped.hEvent);

            if (ReadDirectoryChangesW(
                dir_handle,
                buffer.data(),
                static_cast<DWORD>(buffer.size()),
                TRUE,
                FILE_NOTIFY_CHANGE_FILE_NAME | 
                FILE_NOTIFY_CHANGE_DIR_NAME |
                FILE_NOTIFY_CHANGE_ATTRIBUTES |
                FILE_NOTIFY_CHANGE_SIZE |
                FILE_NOTIFY_CHANGE_LAST_WRITE,
                &bytes_returned,
                &overlapped,
                NULL
            )) {
                WaitForSingleObject(overlapped.hEvent, 1000);

                if (bytes_returned > 0) {
                    process_win32_events(buffer.data(), bytes_returned);
                }
            }
        }

        CloseHandle(overlapped.hEvent);
        CloseHandle(dir_handle);
#else
        int inotify_fd = inotify_init();
        if (inotify_fd < 0) {
            return;
        }

        int watch_desc = inotify_add_watch(
            inotify_fd,
            watch_path_.c_str(),
            IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO
        );

        if (watch_desc < 0) {
            close(inotify_fd);
            return;
        }

        std::vector<char> buffer(4096 * 10);

        while (running_) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(inotify_fd, &fds);

            struct timeval timeout = {1, 0};

            int ready = select(inotify_fd + 1, &fds, NULL, NULL, &timeout);
            
            if (ready > 0 && FD_ISSET(inotify_fd, &fds)) {
                ssize_t length = read(inotify_fd, buffer.data(), buffer.size());
                if (length > 0) {
                    process_inotify_events(buffer.data(), length);
                }
            }
        }

        inotify_rm_watch(inotify_fd, watch_desc);
        close(inotify_fd);
#endif
    }

#ifdef _WIN32
    void process_win32_events(const char* buffer, DWORD size) {
        const FILE_NOTIFY_INFORMATION* notify_info = 
            reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(buffer);
        
        std::lock_guard<std::mutex> lock(events_mutex_);
        
        while (true) {
            std::wstring wfilename(notify_info->FileName, 
                                  notify_info->FileNameLength / sizeof(WCHAR));
            std::filesystem::path full_path = watch_path_ / wfilename;

            EventType event_type = EventType::Modified;
            
            switch (notify_info->Action) {
                case FILE_ACTION_ADDED:
                    event_type = EventType::Created;
                    break;
                case FILE_ACTION_REMOVED:
                    event_type = EventType::Deleted;
                    break;
                case FILE_ACTION_MODIFIED:
                    event_type = EventType::Modified;
                    break;
                case FILE_ACTION_RENAMED_OLD_NAME:
                case FILE_ACTION_RENAMED_NEW_NAME:
                    event_type = EventType::Renamed;
                    break;
            }

            events_.push_back({
                full_path,
                event_type,
                std::chrono::system_clock::now()
            });

            if (notify_info->NextEntryOffset == 0) {
                break;
            }
            
            notify_info = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(
                reinterpret_cast<const char*>(notify_info) + 
                notify_info->NextEntryOffset
            );
        }
    }
#else
    void process_inotify_events(const char* buffer, ssize_t length) {
        std::lock_guard<std::mutex> lock(events_mutex_);
        
        const struct inotify_event* event;
        
        for (const char* ptr = buffer; ptr < buffer + length; 
             ptr += sizeof(struct inotify_event) + event->len) {
            
            event = reinterpret_cast<const struct inotify_event*>(ptr);
            
            if (event->len > 0) {
                std::filesystem::path full_path = watch_path_ / event->name;
                EventType event_type = EventType::Modified;
                
                if (event->mask & IN_CREATE) {
                    event_type = EventType::Created;
                } else if (event->mask & IN_DELETE) {
                    event_type = EventType::Deleted;
                } else if (event->mask & IN_MODIFY) {
                    event_type = EventType::Modified;
                } else if (event->mask & (IN_MOVED_FROM | IN_MOVED_TO)) {
                    event_type = EventType::Renamed;
                }

                events_.push_back({
                    full_path,
                    event_type,
                    std::chrono::system_clock::now()
                });
            }
        }
    }
#endif

    std::filesystem::path watch_path_;
    std::atomic<bool> running_;
    std::thread watch_thread_;
    std::vector<FileEvent> events_;
    std::mutex events_mutex_;
};