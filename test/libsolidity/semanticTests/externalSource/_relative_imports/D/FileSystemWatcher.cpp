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
#endif

class FileSystemWatcher {
public:
    FileSystemWatcher(const std::string& path) : watchPath(path), running(false) {}

    ~FileSystemWatcher() {
        stop();
    }

    bool start() {
        if (running) return false;
        running = true;
        watcherThread = std::thread(&FileSystemWatcher::watchLoop, this);
        return true;
    }

    void stop() {
        running = false;
        if (watcherThread.joinable()) {
            watcherThread.join();
        }
    }

    void addCallback(std::function<void(const std::string&, const std::string&)> callback) {
        callbacks.push_back(callback);
    }

private:
    std::string watchPath;
    std::atomic<bool> running;
    std::thread watcherThread;
    std::vector<std::function<void(const std::string&, const std::string&)>> callbacks;

    void watchLoop() {
#ifdef _WIN32
        HANDLE dirHandle = CreateFileA(
            watchPath.c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            NULL
        );

        if (dirHandle == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to open directory: " << watchPath << std::endl;
            return;
        }

        char buffer[1024];
        DWORD bytesReturned;

        while (running) {
            if (ReadDirectoryChangesW(
                dirHandle,
                buffer,
                sizeof(buffer),
                TRUE,
                FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
                &bytesReturned,
                NULL,
                NULL)) {

                FILE_NOTIFY_INFORMATION* notifyInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
                do {
                    std::wstring wideFileName(notifyInfo->FileName, notifyInfo->FileNameLength / sizeof(WCHAR));
                    std::string fileName(wideFileName.begin(), wideFileName.end());
                    std::string eventType;

                    switch (notifyInfo->Action) {
                        case FILE_ACTION_ADDED: eventType = "ADDED"; break;
                        case FILE_ACTION_REMOVED: eventType = "REMOVED"; break;
                        case FILE_ACTION_MODIFIED: eventType = "MODIFIED"; break;
                        case FILE_ACTION_RENAMED_OLD_NAME: eventType = "RENAMED_OLD"; break;
                        case FILE_ACTION_RENAMED_NEW_NAME: eventType = "RENAMED_NEW"; break;
                        default: eventType = "UNKNOWN"; break;
                    }

                    for (auto& callback : callbacks) {
                        callback(fileName, eventType);
                    }

                    if (notifyInfo->NextEntryOffset == 0) break;
                    notifyInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<BYTE*>(notifyInfo) + notifyInfo->NextEntryOffset);
                } while (true);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        CloseHandle(dirHandle);
#else
        int inotifyFd = inotify_init();
        if (inotifyFd < 0) {
            std::cerr << "Failed to initialize inotify" << std::endl;
            return;
        }

        int watchDescriptor = inotify_add_watch(inotifyFd, watchPath.c_str(), IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO);
        if (watchDescriptor < 0) {
            std::cerr << "Failed to add watch for: " << watchPath << std::endl;
            close(inotifyFd);
            return;
        }

        char buffer[1024];
        while (running) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(inotifyFd, &fds);

            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            int ready = select(inotifyFd + 1, &fds, NULL, NULL, &timeout);
            if (ready > 0 && FD_ISSET(inotifyFd, &fds)) {
                int length = read(inotifyFd, buffer, sizeof(buffer));
                if (length < 0) continue;

                int i = 0;
                while (i < length) {
                    struct inotify_event* event = reinterpret_cast<struct inotify_event*>(&buffer[i]);
                    if (event->len) {
                        std::string fileName(event->name);
                        std::string eventType;

                        if (event->mask & IN_CREATE) eventType = "CREATED";
                        else if (event->mask & IN_DELETE) eventType = "DELETED";
                        else if (event->mask & IN_MODIFY) eventType = "MODIFIED";
                        else if (event->mask & IN_MOVED_FROM) eventType = "MOVED_FROM";
                        else if (event->mask & IN_MOVED_TO) eventType = "MOVED_TO";

                        for (auto& callback : callbacks) {
                            callback(fileName, eventType);
                        }
                    }
                    i += sizeof(struct inotify_event) + event->len;
                }
            }
        }

        inotify_rm_watch(inotifyFd, watchDescriptor);
        close(inotifyFd);
#endif
    }
};