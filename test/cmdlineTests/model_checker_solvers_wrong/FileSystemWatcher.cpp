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

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const std::string& path) : watchPath(path), running(false) {
        if (!fs::exists(path) || !fs::is_directory(path)) {
            throw std::runtime_error("Invalid directory path");
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
        #ifdef _WIN32
            if (directoryHandle != INVALID_HANDLE_VALUE) {
                CloseHandle(directoryHandle);
                directoryHandle = INVALID_HANDLE_VALUE;
            }
        #else
            if (inotifyFd >= 0) {
                close(inotifyFd);
                inotifyFd = -1;
            }
        #endif
    }

    void setEventCallback(std::function<void(const std::string&, uint32_t)> callback) {
        eventCallback = callback;
    }

private:
    std::string watchPath;
    std::atomic<bool> running;
    std::thread watcherThread;
    std::function<void(const std::string&, uint32_t)> eventCallback;

    #ifdef _WIN32
        HANDLE directoryHandle = INVALID_HANDLE_VALUE;
    #else
        int inotifyFd = -1;
        int watchDescriptor = -1;
    #endif

    void watchLoop() {
        #ifdef _WIN32
            watchLoopWindows();
        #else
            watchLoopLinux();
        #endif
    }

    #ifdef _WIN32
    void watchLoopWindows() {
        directoryHandle = CreateFileA(
            watchPath.c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            NULL
        );

        if (directoryHandle == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to open directory for watching" << std::endl;
            return;
        }

        char buffer[4096];
        DWORD bytesReturned;

        while (running) {
            if (ReadDirectoryChangesW(
                directoryHandle,
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
                FILE_NOTIFY_INFORMATION* notifyInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
                
                do {
                    std::wstring wideFilename(notifyInfo->FileName, notifyInfo->FileNameLength / sizeof(WCHAR));
                    std::string filename(wideFilename.begin(), wideFilename.end());
                    std::string fullPath = watchPath + "\\" + filename;

                    if (eventCallback) {
                        eventCallback(fullPath, notifyInfo->Action);
                    }

                    if (notifyInfo->NextEntryOffset == 0) break;
                    notifyInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                        reinterpret_cast<BYTE*>(notifyInfo) + notifyInfo->NextEntryOffset
                    );
                } while (true);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    #else
    void watchLoopLinux() {
        inotifyFd = inotify_init();
        if (inotifyFd < 0) {
            std::cerr << "Failed to initialize inotify" << std::endl;
            return;
        }

        watchDescriptor = inotify_add_watch(
            inotifyFd,
            watchPath.c_str(),
            IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO
        );

        if (watchDescriptor < 0) {
            std::cerr << "Failed to add watch for directory" << std::endl;
            close(inotifyFd);
            inotifyFd = -1;
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
                if (length <= 0) continue;

                for (char* ptr = buffer; ptr < buffer + length; ) {
                    event = reinterpret_cast<const struct inotify_event*>(ptr);
                    
                    if (event->len) {
                        std::string filename(event->name);
                        std::string fullPath = watchPath + "/" + filename;
                        
                        if (eventCallback) {
                            eventCallback(fullPath, event->mask);
                        }
                    }
                    ptr += sizeof(struct inotify_event) + event->len;
                }
            }
        }
    }
    #endif
};