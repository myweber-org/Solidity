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
#include <limits.h>
#endif

class FileSystemWatcher {
public:
    FileSystemWatcher(const std::string& path) : watchPath(path), running(false) {
        if (!std::filesystem::exists(path)) {
            throw std::runtime_error("Path does not exist: " + path);
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

    void start() {
        running = true;
        watcherThread = std::thread(&FileSystemWatcher::watchLoop, this);
    }

    void stop() {
        running = false;
        if (watcherThread.joinable()) {
            watcherThread.join();
        }
    }

    void setCallback(std::function<void(const std::string&, const std::string&)> cb) {
        callback = cb;
    }

private:
    std::string watchPath;
    std::atomic<bool> running;
    std::thread watcherThread;
    std::function<void(const std::string&, const std::string&)> callback;

    void watchLoop() {
#ifdef _WIN32
        watchWindows();
#else
        watchLinux();
#endif
    }

#ifdef _WIN32
    void watchWindows() {
        HANDLE hDir = CreateFileA(
            watchPath.c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            NULL
        );

        if (hDir == INVALID_HANDLE_VALUE) return;

        char buffer[4096];
        DWORD bytesReturned;
        OVERLAPPED overlapped = {0};
        overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        while (running) {
            if (ReadDirectoryChangesW(
                hDir,
                buffer,
                sizeof(buffer),
                TRUE,
                FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
                FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
                &bytesReturned,
                &overlapped,
                NULL)) {

                WaitForSingleObject(overlapped.hEvent, INFINITE);
                if (!running) break;

                FILE_NOTIFY_INFORMATION* notifyInfo = (FILE_NOTIFY_INFORMATION*)buffer;
                do {
                    std::wstring wideName(notifyInfo->FileName, notifyInfo->FileNameLength / sizeof(WCHAR));
                    std::string fileName(wideName.begin(), wideName.end());
                    std::string action;

                    switch (notifyInfo->Action) {
                        case FILE_ACTION_ADDED: action = "ADDED"; break;
                        case FILE_ACTION_REMOVED: action = "REMOVED"; break;
                        case FILE_ACTION_MODIFIED: action = "MODIFIED"; break;
                        case FILE_ACTION_RENAMED_OLD_NAME: action = "RENAMED_OLD"; break;
                        case FILE_ACTION_RENAMED_NEW_NAME: action = "RENAMED_NEW"; break;
                        default: action = "UNKNOWN";
                    }

                    if (callback) {
                        callback(fileName, action);
                    }

                    if (notifyInfo->NextEntryOffset == 0) break;
                    notifyInfo = (FILE_NOTIFY_INFORMATION*)((BYTE*)notifyInfo + notifyInfo->NextEntryOffset);
                } while (true);

                ResetEvent(overlapped.hEvent);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        CloseHandle(overlapped.hEvent);
        CloseHandle(hDir);
    }
#else
    void watchLinux() {
        int inotifyFd = inotify_init();
        if (inotifyFd < 0) return;

        int watchDesc = inotify_add_watch(inotifyFd, watchPath.c_str(),
                                         IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO);
        if (watchDesc < 0) {
            close(inotifyFd);
            return;
        }

        char buffer[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
        const struct inotify_event* event;

        while (running) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(inotifyFd, &fds);

            struct timeval timeout = {1, 0};
            int ready = select(inotifyFd + 1, &fds, NULL, NULL, &timeout);

            if (ready > 0 && FD_ISSET(inotifyFd, &fds)) {
                ssize_t len = read(inotifyFd, buffer, sizeof(buffer));
                if (len <= 0) continue;

                for (char* ptr = buffer; ptr < buffer + len; ptr += sizeof(struct inotify_event) + event->len) {
                    event = (const struct inotify_event*)ptr;
                    if (event->len == 0) continue;

                    std::string fileName(event->name);
                    std::string action;

                    if (event->mask & IN_CREATE) action = "CREATED";
                    else if (event->mask & IN_DELETE) action = "DELETED";
                    else if (event->mask & IN_MODIFY) action = "MODIFIED";
                    else if (event->mask & IN_MOVED_FROM) action = "MOVED_FROM";
                    else if (event->mask & IN_MOVED_TO) action = "MOVED_TO";

                    if (!action.empty() && callback) {
                        callback(fileName, action);
                    }
                }
            }
        }

        inotify_rm_watch(inotifyFd, watchDesc);
        close(inotifyFd);
    }
#endif
};