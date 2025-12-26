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

private:
    std::string watchPath;
    std::atomic<bool> running;
    std::thread watcherThread;

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

        if (hDir == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to open directory: " << watchPath << std::endl;
            return;
        }

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
                while (notifyInfo != NULL) {
                    processWindowsEvent(notifyInfo);
                    if (notifyInfo->NextEntryOffset == 0) break;
                    notifyInfo = (FILE_NOTIFY_INFORMATION*)((BYTE*)notifyInfo + notifyInfo->NextEntryOffset);
                }
                ResetEvent(overlapped.hEvent);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        CloseHandle(overlapped.hEvent);
        CloseHandle(hDir);
    }

    void processWindowsEvent(FILE_NOTIFY_INFORMATION* notifyInfo) {
        std::wstring wideFilename(notifyInfo->FileName, notifyInfo->FileNameLength / sizeof(WCHAR));
        std::string filename(wideFilename.begin(), wideFilename.end());
        std::string fullPath = watchPath + "\\" + filename;

        switch (notifyInfo->Action) {
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
    }
#else
    void watchLinux() {
        int inotifyFd = inotify_init();
        if (inotifyFd < 0) {
            std::cerr << "Failed to initialize inotify" << std::endl;
            return;
        }

        int watchDescriptor = inotify_add_watch(inotifyFd, watchPath.c_str(),
            IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO);

        if (watchDescriptor < 0) {
            std::cerr << "Failed to add watch for: " << watchPath << std::endl;
            close(inotifyFd);
            return;
        }

        char buffer[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
        const struct inotify_event* event;

        while (running) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(inotifyFd, &fds);

            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            int ready = select(inotifyFd + 1, &fds, NULL, NULL, &timeout);
            if (ready > 0 && FD_ISSET(inotifyFd, &fds)) {
                ssize_t length = read(inotifyFd, buffer, sizeof(buffer));
                if (length > 0) {
                    for (char* ptr = buffer; ptr < buffer + length; ptr += sizeof(struct inotify_event) + event->len) {
                        event = (const struct inotify_event*)ptr;
                        processLinuxEvent(event);
                    }
                }
            }
        }

        inotify_rm_watch(inotifyFd, watchDescriptor);
        close(inotifyFd);
    }

    void processLinuxEvent(const struct inotify_event* event) {
        std::string filename = event->name;
        std::string fullPath = watchPath + "/" + filename;

        if (event->mask & IN_CREATE) {
            std::cout << "File created: " << fullPath << std::endl;
        } else if (event->mask & IN_DELETE) {
            std::cout << "File deleted: " << fullPath << std::endl;
        } else if (event->mask & IN_MODIFY) {
            std::cout << "File modified: " << fullPath << std::endl;
        } else if (event->mask & IN_MOVED_FROM) {
            std::cout << "File moved from: " << fullPath << std::endl;
        } else if (event->mask & IN_MOVED_TO) {
            std::cout << "File moved to: " << fullPath << std::endl;
        }
    }
#endif
};

int main() {
    std::string pathToWatch = ".";
    
    if (!std::filesystem::exists(pathToWatch)) {
        std::cerr << "Path does not exist: " << pathToWatch << std::endl;
        return 1;
    }

    FileSystemWatcher watcher(pathToWatch);
    std::cout << "Starting file system watcher for: " << pathToWatch << std::endl;
    std::cout << "Press Enter to stop..." << std::endl;

    if (watcher.start()) {
        std::cin.get();
        watcher.stop();
    }

    return 0;
}