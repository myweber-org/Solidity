
#include <iostream>
#include <string>
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
    FileSystemWatcher(const std::string& path) : watchPath(path), running(false) {
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
    std::string watchPath;
    std::atomic<bool> running;
    std::thread watcherThread;
    std::function<void(const std::string&, const std::string&)> callback;

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

        char buffer[1024];
        DWORD bytesReturned;
        FILE_NOTIFY_INFORMATION* pNotify;

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
                NULL)) {

                pNotify = (FILE_NOTIFY_INFORMATION*)buffer;
                do {
                    std::wstring wfilename(pNotify->FileName, pNotify->FileNameLength / sizeof(WCHAR));
                    std::string filename(wfilename.begin(), wfilename.end());
                    std::string fullPath = watchPath + "\\" + filename;

                    std::string action;
                    switch (pNotify->Action) {
                        case FILE_ACTION_ADDED: action = "ADDED"; break;
                        case FILE_ACTION_REMOVED: action = "REMOVED"; break;
                        case FILE_ACTION_MODIFIED: action = "MODIFIED"; break;
                        case FILE_ACTION_RENAMED_OLD_NAME: action = "RENAMED_OLD"; break;
                        case FILE_ACTION_RENAMED_NEW_NAME: action = "RENAMED_NEW"; break;
                        default: action = "UNKNOWN";
                    }

                    if (callback) {
                        callback(fullPath, action);
                    }

                    if (pNotify->NextEntryOffset == 0) break;
                    pNotify = (FILE_NOTIFY_INFORMATION*)((LPBYTE)pNotify + pNotify->NextEntryOffset);
                } while (true);
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

        char buffer[1024 * (sizeof(struct inotify_event) + NAME_MAX + 1)];
        struct inotify_event* event;

        while (running) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(fd, &fds);

            struct timeval timeout = {1, 0};
            int ret = select(fd + 1, &fds, NULL, NULL, &timeout);

            if (ret > 0 && FD_ISSET(fd, &fds)) {
                ssize_t len = read(fd, buffer, sizeof(buffer));
                if (len <= 0) continue;

                ssize_t i = 0;
                while (i < len) {
                    event = (struct inotify_event*)&buffer[i];
                    if (event->len) {
                        std::string filename(event->name);
                        std::string fullPath = watchPath + "/" + filename;

                        std::string action;
                        if (event->mask & IN_CREATE) action = "CREATED";
                        else if (event->mask & IN_DELETE) action = "DELETED";
                        else if (event->mask & IN_MODIFY) action = "MODIFIED";
                        else if (event->mask & IN_MOVED_FROM) action = "MOVED_FROM";
                        else if (event->mask & IN_MOVED_TO) action = "MOVED_TO";

                        if (callback && !action.empty()) {
                            callback(fullPath, action);
                        }
                    }
                    i += sizeof(struct inotify_event) + event->len;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        inotify_rm_watch(fd, wd);
        close(fd);
#endif
    }
};