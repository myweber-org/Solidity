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
            std::cerr << "Failed to open directory for watching" << std::endl;
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
                NULL
            )) {
                FILE_NOTIFY_INFORMATION* notifyInfo = (FILE_NOTIFY_INFORMATION*)buffer;
                
                do {
                    std::wstring wideFileName(notifyInfo->FileName, notifyInfo->FileNameLength / sizeof(WCHAR));
                    std::string fileName(wideFileName.begin(), wideFileName.end());
                    std::string fullPath = watchPath + "\\" + fileName;

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
                        callback(action, fullPath);
                    }

                    if (notifyInfo->NextEntryOffset == 0) break;
                    notifyInfo = (FILE_NOTIFY_INFORMATION*)((BYTE*)notifyInfo + notifyInfo->NextEntryOffset);
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

        int watchDescriptor = inotify_add_watch(inotifyFd, watchPath.c_str(), 
            IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO);

        if (watchDescriptor < 0) {
            std::cerr << "Failed to add watch for directory" << std::endl;
            close(inotifyFd);
            return;
        }

        char buffer[1024 * (sizeof(struct inotify_event) + NAME_MAX + 1)];

        while (running) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(inotifyFd, &fds);

            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            int selectResult = select(inotifyFd + 1, &fds, NULL, NULL, &timeout);
            
            if (selectResult > 0 && FD_ISSET(inotifyFd, &fds)) {
                ssize_t length = read(inotifyFd, buffer, sizeof(buffer));
                
                if (length > 0) {
                    ssize_t i = 0;
                    while (i < length) {
                        struct inotify_event* event = (struct inotify_event*)&buffer[i];
                        
                        if (event->len > 0) {
                            std::string fileName(event->name);
                            std::string fullPath = watchPath + "/" + fileName;
                            std::string action;

                            if (event->mask & IN_CREATE) action = "CREATED";
                            else if (event->mask & IN_DELETE) action = "DELETED";
                            else if (event->mask & IN_MODIFY) action = "MODIFIED";
                            else if (event->mask & IN_MOVED_FROM) action = "MOVED_FROM";
                            else if (event->mask & IN_MOVED_TO) action = "MOVED_TO";

                            if (!action.empty() && callback) {
                                callback(action, fullPath);
                            }
                        }
                        
                        i += sizeof(struct inotify_event) + event->len;
                    }
                }
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        inotify_rm_watch(inotifyFd, watchDescriptor);
        close(inotifyFd);
#endif
    }

    std::string watchPath;
    std::atomic<bool> running;
    std::thread watcherThread;
    std::function<void(const std::string&, const std::string&)> callback;
};