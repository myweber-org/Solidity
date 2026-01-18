
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    SimpleFileWatcher(const std::string& path) : watch_path(path) {
        if (fs::exists(path) && fs::is_directory(path)) {
            refresh_file_map();
        }
    }

    void start_watching(int interval_seconds = 2) {
        std::cout << "Watching directory: " << watch_path << std::endl;
        std::cout << "Polling interval: " << interval_seconds << " seconds" << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    std::string watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_map;

    void refresh_file_map() {
        file_map.clear();
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_map[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void check_for_changes() {
        auto current_files = file_map;
        bool changes_detected = false;

        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (!fs::is_regular_file(entry.path())) {
                continue;
            }

            std::string file_path = entry.path().string();
            auto current_time = fs::last_write_time(entry.path());

            if (file_map.find(file_path) == file_map.end()) {
                std::cout << "[NEW] " << file_path << std::endl;
                changes_detected = true;
            } else if (file_map[file_path] != current_time) {
                std::cout << "[MODIFIED] " << file_path << std::endl;
                changes_detected = true;
            }
            current_files[file_path] = current_time;
        }

        for (const auto& [file_path, _] : file_map) {
            if (!fs::exists(file_path)) {
                std::cout << "[DELETED] " << file_path << std::endl;
                changes_detected = true;
            }
        }

        if (changes_detected) {
            file_map = std::move(current_files);
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        SimpleFileWatcher watcher(argv[1]);
        watcher.start_watching();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}#include <iostream>
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
        HANDLE dir_handle = CreateFileW(
            watch_path_.c_str(),
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

        char buffer[1024];
        DWORD bytes_returned;
        FILE_NOTIFY_INFORMATION* notify_info;

        while (running_) {
            if (ReadDirectoryChangesW(
                dir_handle,
                buffer,
                sizeof(buffer),
                TRUE,
                FILE_NOTIFY_CHANGE_FILE_NAME | 
                FILE_NOTIFY_CHANGE_DIR_NAME |
                FILE_NOTIFY_CHANGE_LAST_WRITE,
                &bytes_returned,
                NULL,
                NULL)) {

                notify_info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
                
                do {
                    std::wstring filename(notify_info->FileName, 
                                         notify_info->FileNameLength / sizeof(WCHAR));
                    std::filesystem::path full_path = watch_path_ / filename;

                    EventType event_type;
                    switch (notify_info->Action) {
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

                    FileEvent event{
                        full_path,
                        event_type,
                        std::chrono::system_clock::now()
                    };

                    {
                        std::lock_guard<std::mutex> lock(events_mutex_);
                        events_.push_back(event);
                    }

                    if (notify_info->NextEntryOffset == 0) break;
                    notify_info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                        reinterpret_cast<BYTE*>(notify_info) + notify_info->NextEntryOffset
                    );
                } while (true);
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        CloseHandle(dir_handle);
#else
        int inotify_fd = inotify_init1(IN_NONBLOCK);
        if (inotify_fd < 0) {
            return;
        }

        int watch_desc = inotify_add_watch(
            inotify_fd,
            watch_path_.c_str(),
            IN_CREATE | IN_MODIFY | IN_DELETE | IN_MOVE
        );

        if (watch_desc < 0) {
            close(inotify_fd);
            return;
        }

        char buffer[1024];
        struct inotify_event* event;

        while (running_) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(inotify_fd, &fds);

            struct timeval timeout = {0, 100000}; // 100ms

            int ready = select(inotify_fd + 1, &fds, NULL, NULL, &timeout);
            
            if (ready > 0 && FD_ISSET(inotify_fd, &fds)) {
                int length = read(inotify_fd, buffer, sizeof(buffer));
                
                if (length > 0) {
                    int i = 0;
                    while (i < length) {
                        event = reinterpret_cast<struct inotify_event*>(&buffer[i]);
                        
                        if (event->len) {
                            std::filesystem::path full_path = watch_path_ / event->name;
                            
                            EventType event_type;
                            if (event->mask & IN_CREATE) {
                                event_type = EventType::Created;
                            } else if (event->mask & IN_MODIFY) {
                                event_type = EventType::Modified;
                            } else if (event->mask & IN_DELETE) {
                                event_type = EventType::Deleted;
                            } else if (event->mask & IN_MOVE) {
                                event_type = EventType::Renamed;
                            } else {
                                i += sizeof(struct inotify_event) + event->len;
                                continue;
                            }

                            FileEvent file_event{
                                full_path,
                                event_type,
                                std::chrono::system_clock::now()
                            };

                            {
                                std::lock_guard<std::mutex> lock(events_mutex_);
                                events_.push_back(file_event);
                            }
                        }
                        
                        i += sizeof(struct inotify_event) + event->len;
                    }
                }
            }
        }

        inotify_rm_watch(inotify_fd, watch_desc);
        close(inotify_fd);
#endif
    }

    std::filesystem::path watch_path_;
    std::atomic<bool> running_;
    std::thread watch_thread_;
    std::vector<FileEvent> events_;
    std::mutex events_mutex_;
};