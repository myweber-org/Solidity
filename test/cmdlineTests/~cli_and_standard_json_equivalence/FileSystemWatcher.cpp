#include <iostream>
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
#include <errno.h>
#endif

class FileSystemWatcher {
public:
    FileSystemWatcher(const std::string& path) : watch_path(path), running(false) {
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
        watch_windows();
#else
        watch_linux();
#endif
    }

#ifdef _WIN32
    void watch_windows() {
        HANDLE dir_handle = CreateFileA(
            watch_path.c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            NULL
        );

        if (dir_handle == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to open directory: " << watch_path << std::endl;
            return;
        }

        char buffer[1024];
        DWORD bytes_returned;
        OVERLAPPED overlapped = {0};
        overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

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
                NULL
            )) {
                WaitForSingleObject(overlapped.hEvent, INFINITE);
                ResetEvent(overlapped.hEvent);

                if (!running) break;

                FILE_NOTIFY_INFORMATION* notify_info = (FILE_NOTIFY_INFORMATION*)buffer;
                while (true) {
                    std::wstring wfilename(notify_info->FileName, notify_info->FileNameLength / sizeof(WCHAR));
                    std::string filename(wfilename.begin(), wfilename.end());
                    std::string full_path = watch_path + "\\" + filename;

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
                        callback(full_path, action);
                    }

                    if (notify_info->NextEntryOffset == 0) break;
                    notify_info = (FILE_NOTIFY_INFORMATION*)((BYTE*)notify_info + notify_info->NextEntryOffset);
                }
            }
        }

        CloseHandle(overlapped.hEvent);
        CloseHandle(dir_handle);
    }
#else
    void watch_linux() {
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

        char buffer[1024];
        while (running) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(inotify_fd, &fds);

            struct timeval timeout = {1, 0};
            int ret = select(inotify_fd + 1, &fds, NULL, NULL, &timeout);

            if (ret > 0 && FD_ISSET(inotify_fd, &fds)) {
                int length = read(inotify_fd, buffer, sizeof(buffer));
                if (length < 0) continue;

                int i = 0;
                while (i < length) {
                    struct inotify_event* event = (struct inotify_event*)&buffer[i];
                    if (event->len) {
                        std::string filename(event->name);
                        std::string full_path = watch_path + "/" + filename;
                        std::string action;

                        if (event->mask & IN_CREATE) action = "CREATED";
                        else if (event->mask & IN_DELETE) action = "DELETED";
                        else if (event->mask & IN_MODIFY) action = "MODIFIED";
                        else if (event->mask & IN_MOVED_FROM) action = "MOVED_FROM";
                        else if (event->mask & IN_MOVED_TO) action = "MOVED_TO";

                        if (!action.empty() && callback) {
                            callback(full_path, action);
                        }
                    }
                    i += sizeof(struct inotify_event) + event->len;
                }
            }
        }

        inotify_rm_watch(inotify_fd, watch_desc);
        close(inotify_fd);
    }
#endif
};