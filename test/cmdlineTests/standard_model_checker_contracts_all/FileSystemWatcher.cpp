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
#include <fcntl.h>
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

        if (dir_handle == INVALID_HANDLE_VALUE) return;

        std::vector<BYTE> buffer(4096);
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
                FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
                FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
                &bytes_returned,
                &overlapped,
                NULL
            ) == 0) {
                break;
            }

            DWORD wait_result = WaitForSingleObject(overlapped.hEvent, 100);
            if (wait_result == WAIT_OBJECT_0) {
                process_windows_events(buffer.data(), bytes_returned);
            }
        }

        CloseHandle(overlapped.hEvent);
        CloseHandle(dir_handle);
    }

    void process_windows_events(BYTE* buffer, DWORD size) {
        FILE_NOTIFY_INFORMATION* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
        
        while (info != nullptr) {
            std::wstring filename(info->FileName, info->FileNameLength / sizeof(WCHAR));
            std::filesystem::path full_path = watch_path_ / filename;

            EventType event_type;
            switch (info->Action) {
                case FILE_ACTION_ADDED:
                    event_type = EventType::Created;
                    break;
                case FILE_ACTION_MODIFIED:
                    event_type = EventType::Modified;
                    break;
                case FILE_ACTION_REMOVED:
                    event_type = EventType::Deleted;
                    break;
                case FILE_ACTION_RENAMED_OLD_NAME:
                case FILE_ACTION_RENAMED_NEW_NAME:
                    event_type = EventType::Renamed;
                    break;
                default:
                    continue;
            }

            add_event({full_path, event_type, std::chrono::system_clock::now()});

            if (info->NextEntryOffset == 0) break;
            info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                reinterpret_cast<BYTE*>(info) + info->NextEntryOffset
            );
        }
    }
#else
    void watch_linux() {
        int inotify_fd = inotify_init1(IN_NONBLOCK);
        if (inotify_fd < 0) return;

        int watch_desc = inotify_add_watch(
            inotify_fd,
            watch_path_.c_str(),
            IN_CREATE | IN_MODIFY | IN_DELETE | IN_MOVE
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

            struct timeval timeout = {0, 100000}; // 100ms
            int ready = select(inotify_fd + 1, &fds, NULL, NULL, &timeout);
            
            if (ready > 0 && FD_ISSET(inotify_fd, &fds)) {
                ssize_t len = read(inotify_fd, buffer, sizeof(buffer));
                if (len > 0) {
                    process_linux_events(buffer, len);
                }
            }
        }

        inotify_rm_watch(inotify_fd, watch_desc);
        close(inotify_fd);
    }

    void process_linux_events(char* buffer, ssize_t len) {
        const struct inotify_event* event;
        
        for (char* ptr = buffer; ptr < buffer + len; 
             ptr += sizeof(struct inotify_event) + event->len) {
            event = reinterpret_cast<const struct inotify_event*>(ptr);
            
            if (event->mask & IN_IGNORED) continue;
            
            std::string filename(event->name);
            std::filesystem::path full_path = watch_path_ / filename;

            EventType event_type;
            if (event->mask & (IN_CREATE | IN_MOVED_TO)) {
                event_type = EventType::Created;
            } else if (event->mask & IN_MODIFY) {
                event_type = EventType::Modified;
            } else if (event->mask & (IN_DELETE | IN_MOVED_FROM)) {
                event_type = EventType::Deleted;
            } else {
                continue;
            }

            add_event({full_path, event_type, std::chrono::system_clock::now()});
        }
    }
#endif

    void add_event(const FileEvent& event) {
        std::lock_guard<std::mutex> lock(events_mutex_);
        events_.push_back(event);
    }

    std::filesystem::path watch_path_;
    std::atomic<bool> running_;
    std::thread watch_thread_;
    std::vector<FileEvent> events_;
    std::mutex events_mutex_;
};

void print_events(const std::vector<FileSystemWatcher::FileEvent>& events) {
    for (const auto& event : events) {
        std::string type_str;
        switch (event.type) {
            case FileSystemWatcher::EventType::Created:
                type_str = "CREATED";
                break;
            case FileSystemWatcher::EventType::Modified:
                type_str = "MODIFIED";
                break;
            case FileSystemWatcher::EventType::Deleted:
                type_str = "DELETED";
                break;
            case FileSystemWatcher::EventType::Renamed:
                type_str = "RENAMED";
                break;
        }
        
        auto time_t = std::chrono::system_clock::to_time_t(event.timestamp);
        std::cout << "[" << std::ctime(&time_t) << "] "
                  << type_str << ": " << event.path.string() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start();

        std::cout << "Watching directory: " << argv[1] << std::endl;
        std::cout << "Press Enter to stop..." << std::endl;

        while (true) {
            auto events = watcher.get_events();
            if (!events.empty()) {
                print_events(events);
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            if (std::cin.peek() != EOF) {
                std::cin.get();
                break;
            }
        }

        watcher.stop();
        std::cout << "Stopped watching." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}