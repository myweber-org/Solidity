
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
#include <errno.h>
#endif

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const std::string& path) : watchPath(path), running(false) {
        if (!std::filesystem::exists(path)) {
            throw std::runtime_error("Path does not exist: " + path);
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

    void start() {
        if (running) return;
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
    void watchLoop() {
#ifdef _WIN32
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
                NULL,
                NULL
            )) {
                FILE_NOTIFY_INFORMATION* notifyInfo = (FILE_NOTIFY_INFORMATION*)buffer;
                while (notifyInfo != NULL) {
                    std::wstring wfilename(notifyInfo->FileName, notifyInfo->FileNameLength / sizeof(WCHAR));
                    std::string filename(wfilename.begin(), wfilename.end());
                    std::string fullPath = watchPath + "\\" + filename;

                    std::string action;
                    switch (notifyInfo->Action) {
                        case FILE_ACTION_ADDED: action = "ADDED"; break;
                        case FILE_ACTION_REMOVED: action = "REMOVED"; break;
                        case FILE_ACTION_MODIFIED: action = "MODIFIED"; break;
                        case FILE_ACTION_RENAMED_OLD_NAME: action = "RENAMED_OLD"; break;
                        case FILE_ACTION_RENAMED_NEW_NAME: action = "RENAMED_NEW"; break;
                        default: action = "UNKNOWN"; break;
                    }

                    if (callback) {
                        callback(fullPath, action);
                    }

                    if (notifyInfo->NextEntryOffset == 0) break;
                    notifyInfo = (FILE_NOTIFY_INFORMATION*)((BYTE*)notifyInfo + notifyInfo->NextEntryOffset);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        CloseHandle(hDir);
#else
        int fd = inotify_init();
        if (fd < 0) return;

        int wd = inotify_add_watch(fd, watchPath.c_str(),
                                  IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO);
        if (wd < 0) {
            close(fd);
            return;
        }

        char buffer[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
        const struct inotify_event* event;

        while (running) {
            ssize_t len = read(fd, buffer, sizeof(buffer));
            if (len <= 0) continue;

            for (char* ptr = buffer; ptr < buffer + len; ptr += sizeof(struct inotify_event) + event->len) {
                event = (const struct inotify_event*)ptr;

                if (event->len) {
                    std::string filename(event->name);
                    std::string fullPath = watchPath + "/" + filename;
                    std::string action;

                    if (event->mask & IN_CREATE) action = "ADDED";
                    else if (event->mask & IN_DELETE) action = "REMOVED";
                    else if (event->mask & IN_MODIFY) action = "MODIFIED";
                    else if (event->mask & IN_MOVED_FROM) action = "RENAMED_OLD";
                    else if (event->mask & IN_MOVED_TO) action = "RENAMED_NEW";

                    if (callback && !action.empty()) {
                        callback(fullPath, action);
                    }
                }
            }
        }

        inotify_rm_watch(fd, wd);
        close(fd);
#endif
    }

    std::string watchPath;
    std::atomic<bool> running;
    std::thread watcherThread;
    std::function<void(const std::string&, const std::string&)> callback;
};