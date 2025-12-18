
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
        OVERLAPPED overlapped = {0};
        overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        while (running) {
            if (ReadDirectoryChangesW(
                dirHandle,
                buffer,
                sizeof(buffer),
                TRUE,
                FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
                FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
                &bytesReturned,
                &overlapped,
                NULL
            )) {
                WaitForSingleObject(overlapped.hEvent, INFINITE);
                if (bytesReturned > 0) {
                    FILE_NOTIFY_INFORMATION* notifyInfo = (FILE_NOTIFY_INFORMATION*)buffer;
                    do {
                        std::wstring fileName(notifyInfo->FileName, notifyInfo->FileNameLength / sizeof(WCHAR));
                        handleEvent(fileName, notifyInfo->Action);
                        if (notifyInfo->NextEntryOffset == 0) break;
                        notifyInfo = (FILE_NOTIFY_INFORMATION*)((BYTE*)notifyInfo + notifyInfo->NextEntryOffset);
                    } while (true);
                }
                ResetEvent(overlapped.hEvent);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        CloseHandle(overlapped.hEvent);
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
            std::cerr << "Failed to add watch for: " << watchPath << std::endl;
            close(inotifyFd);
            return;
        }

        char eventBuffer[1024 * (sizeof(struct inotify_event) + NAME_MAX + 1)];

        while (running) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(inotifyFd, &fds);

            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            int ready = select(inotifyFd + 1, &fds, NULL, NULL, &timeout);
            if (ready > 0 && FD_ISSET(inotifyFd, &fds)) {
                ssize_t length = read(inotifyFd, eventBuffer, sizeof(eventBuffer));
                if (length > 0) {
                    processInotifyEvents(eventBuffer, length);
                }
            }
        }

        inotify_rm_watch(inotifyFd, watchDescriptor);
        close(inotifyFd);
#endif
    }

#ifdef _WIN32
    void handleEvent(const std::wstring& fileName, DWORD action) {
        std::string actionStr;
        switch (action) {
            case FILE_ACTION_ADDED: actionStr = "ADDED"; break;
            case FILE_ACTION_REMOVED: actionStr = "REMOVED"; break;
            case FILE_ACTION_MODIFIED: actionStr = "MODIFIED"; break;
            case FILE_ACTION_RENAMED_OLD_NAME: actionStr = "RENAMED_OLD"; break;
            case FILE_ACTION_RENAMED_NEW_NAME: actionStr = "RENAMED_NEW"; break;
            default: actionStr = "UNKNOWN"; break;
        }
        std::wcout << L"File: " << fileName << L" Action: " << actionStr.c_str() << std::endl;
    }
#else
    void processInotifyEvents(char* buffer, ssize_t length) {
        struct inotify_event* event;
        for (char* ptr = buffer; ptr < buffer + length; ptr += sizeof(struct inotify_event) + event->len) {
            event = (struct inotify_event*)ptr;
            if (event->len) {
                std::string fileName(event->name);
                std::string eventType;
                if (event->mask & IN_CREATE) eventType = "CREATED";
                if (event->mask & IN_DELETE) eventType = "DELETED";
                if (event->mask & IN_MODIFY) eventType = "MODIFIED";
                if (event->mask & IN_MOVED_FROM) eventType = "MOVED_FROM";
                if (event->mask & IN_MOVED_TO) eventType = "MOVED_TO";
                std::cout << "File: " << fileName << " Event: " << eventType << std::endl;
            }
        }
    }
#endif
};

int main() {
    std::filesystem::path currentPath = std::filesystem::current_path();
    FileSystemWatcher watcher(currentPath.string());
    
    std::cout << "Watching directory: " << currentPath.string() << std::endl;
    std::cout << "Press Enter to stop watching..." << std::endl;
    
    watcher.start();
    std::cin.get();
    watcher.stop();
    
    return 0;
}