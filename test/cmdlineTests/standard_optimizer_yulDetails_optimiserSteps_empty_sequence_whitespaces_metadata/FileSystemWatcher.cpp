
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <filesystem>
#include <atomic>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/inotify.h>
    #include <unistd.h>
    #include <limits.h>
#endif

class FileSystemWatcher {
private:
    std::string watchPath;
    std::atomic<bool> running{false};
    std::thread watchThread;

#ifdef _WIN32
    HANDLE dirHandle = INVALID_HANDLE_VALUE;
    OVERLAPPED overlapped{};
    char buffer[1024];
#else
    int inotifyFd = -1;
    int watchDescriptor = -1;
#endif

    void watchLoop() {
#ifdef _WIN32
        if (dirHandle == INVALID_HANDLE_VALUE) return;

        while (running) {
            DWORD bytesReturned = 0;
            if (GetOverlappedResult(dirHandle, &overlapped, &bytesReturned, FALSE)) {
                if (bytesReturned > 0) {
                    FILE_NOTIFY_INFORMATION* fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
                    while (true) {
                        std::wstring fileNameW(fni->FileName, fni->FileNameLength / sizeof(WCHAR));
                        std::string fileName(fileNameW.begin(), fileNameW.end());
                        std::string fullPath = watchPath + "\\" + fileName;

                        switch (fni->Action) {
                            case FILE_ACTION_ADDED:
                                std::cout << "File added: " << fullPath << std::endl;
                                break;
                            case FILE_ACTION_REMOVED:
                                std::cout << "File removed: " << fullPath << std::endl;
                                break;
                            case FILE_ACTION_MODIFIED:
                                std::cout << "File modified: " << fullPath << std::endl;
                                break;
                            case FILE_ACTION_RENAMED_OLD_NAME:
                                std::cout << "File renamed from: " << fullPath << std::endl;
                                break;
                            case FILE_ACTION_RENAMED_NEW_NAME:
                                std::cout << "File renamed to: " << fullPath << std::endl;
                                break;
                        }

                        if (fni->NextEntryOffset == 0) break;
                        fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<BYTE*>(fni) + fni->NextEntryOffset);
                    }
                }

                memset(&overlapped, 0, sizeof(OVERLAPPED));
                memset(buffer, 0, sizeof(buffer));
                if (!ReadDirectoryChangesW(dirHandle, buffer, sizeof(buffer), FALSE,
                    FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                    FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
                    FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
                    NULL, &overlapped, NULL)) {
                    break;
                }
            }
            Sleep(100);
        }
#else
        char eventBuffer[1024];
        while (running) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(inotifyFd, &fds);

            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            int ret = select(inotifyFd + 1, &fds, NULL, NULL, &timeout);
            if (ret > 0 && FD_ISSET(inotifyFd, &fds)) {
                ssize_t len = read(inotifyFd, eventBuffer, sizeof(eventBuffer));
                if (len < 0) break;

                ssize_t i = 0;
                while (i < len) {
                    struct inotify_event* event = reinterpret_cast<struct inotify_event*>(&eventBuffer[i]);
                    if (event->len) {
                        std::string fileName(event->name);
                        std::string fullPath = watchPath + "/" + fileName;

                        if (event->mask & IN_CREATE) {
                            std::cout << "File created: " << fullPath << std::endl;
                        }
                        if (event->mask & IN_DELETE) {
                            std::cout << "File deleted: " << fullPath << std::endl;
                        }
                        if (event->mask & IN_MODIFY) {
                            std::cout << "File modified: " << fullPath << std::endl;
                        }
                        if (event->mask & IN_MOVED_FROM) {
                            std::cout << "File moved from: " << fullPath << std::endl;
                        }
                        if (event->mask & IN_MOVED_TO) {
                            std::cout << "File moved to: " << fullPath << std::endl;
                        }
                    }
                    i += sizeof(struct inotify_event) + event->len;
                }
            }
        }
#endif
    }

public:
    explicit FileSystemWatcher(const std::string& path) : watchPath(path) {
        std::filesystem::path fsPath(path);
        if (!std::filesystem::exists(fsPath)) {
            throw std::runtime_error("Path does not exist");
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

    bool start() {
        if (running) return false;

#ifdef _WIN32
        dirHandle = CreateFileA(watchPath.c_str(), FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);

        if (dirHandle == INVALID_HANDLE_VALUE) {
            return false;
        }

        memset(&overlapped, 0, sizeof(OVERLAPPED));
        if (!ReadDirectoryChangesW(dirHandle, buffer, sizeof(buffer), FALSE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
            FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
            FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
            NULL, &overlapped, NULL)) {
            CloseHandle(dirHandle);
            dirHandle = INVALID_HANDLE_VALUE;
            return false;
        }
#else
        inotifyFd = inotify_init();
        if (inotifyFd < 0) {
            return false;
        }

        watchDescriptor = inotify_add_watch(inotifyFd, watchPath.c_str(),
            IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO);
        if (watchDescriptor < 0) {
            close(inotifyFd);
            inotifyFd = -1;
            return false;
        }
#endif

        running = true;
        watchThread = std::thread(&FileSystemWatcher::watchLoop, this);
        return true;
    }

    void stop() {
        if (!running) return;

        running = false;
        if (watchThread.joinable()) {
            watchThread.join();
        }

#ifdef _WIN32
        if (dirHandle != INVALID_HANDLE_VALUE) {
            CancelIo(dirHandle);
            CloseHandle(dirHandle);
            dirHandle = INVALID_HANDLE_VALUE;
        }
#else
        if (watchDescriptor >= 0) {
            inotify_rm_watch(inotifyFd, watchDescriptor);
            watchDescriptor = -1;
        }
        if (inotifyFd >= 0) {
            close(inotifyFd);
            inotifyFd = -1;
        }
#endif
    }
};