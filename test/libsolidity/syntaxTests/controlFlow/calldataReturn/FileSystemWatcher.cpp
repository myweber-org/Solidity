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
    #include <cstring>
#endif

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const std::string& path) : watchPath(path), running(false) {
        if (!fs::exists(path) || !fs::is_directory(path)) {
            throw std::runtime_error("Path does not exist or is not a directory");
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

    void setCallback(std::function<void(const std::string&, uint32_t)> cb) {
        callback = std::move(cb);
    }

private:
    std::string watchPath;
    std::atomic<bool> running;
    std::thread watcherThread;
    std::function<void(const std::string&, uint32_t)> callback;

    #ifdef _WIN32
        HANDLE directoryHandle = INVALID_HANDLE_VALUE;
        static const DWORD bufferSize = 65536;
    #else
        int inotifyFd = -1;
        int watchDescriptor = -1;
        static const size_t eventSize = sizeof(struct inotify_event);
        static const size_t bufferSize = 1024 * (eventSize + NAME_MAX + 1);
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
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            nullptr
        );

        if (directoryHandle == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to open directory for watching" << std::endl;
            return;
        }

        BYTE buffer[bufferSize];
        DWORD bytesReturned;
        OVERLAPPED overlapped = {0};
        overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

        while (running) {
            ResetEvent(overlapped.hEvent);
            
            if (!ReadDirectoryChangesW(
                directoryHandle,
                buffer,
                bufferSize,
                TRUE,
                FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
                FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
                &bytesReturned,
                &overlapped,
                nullptr
            )) {
                break;
            }

            DWORD waitResult = WaitForSingleObject(overlapped.hEvent, 1000);
            if (waitResult == WAIT_OBJECT_0) {
                if (!GetOverlappedResult(directoryHandle, &overlapped, &bytesReturned, FALSE)) {
                    break;
                }

                if (bytesReturned > 0) {
                    FILE_NOTIFY_INFORMATION* notifyInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
                    do {
                        std::wstring wideFileName(notifyInfo->FileName, notifyInfo->FileNameLength / sizeof(WCHAR));
                        std::string fileName(wideFileName.begin(), wideFileName.end());
                        std::string fullPath = watchPath + "\\" + fileName;
                        
                        if (callback) {
                            callback(fullPath, notifyInfo->Action);
                        }

                        if (notifyInfo->NextEntryOffset == 0) {
                            break;
                        }
                        notifyInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                            reinterpret_cast<BYTE*>(notifyInfo) + notifyInfo->NextEntryOffset
                        );
                    } while (true);
                }
            }
        }

        CloseHandle(overlapped.hEvent);
    }
    #else
    void watchLoopLinux() {
        inotifyFd = inotify_init();
        if (inotifyFd < 0) {
            std::cerr << "Failed to initialize inotify" << std::endl;
            return;
        }

        watchDescriptor = inotify_add_watch(inotifyFd, watchPath.c_str(),
            IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO);

        if (watchDescriptor < 0) {
            std::cerr << "Failed to add watch for directory" << std::endl;
            close(inotifyFd);
            inotifyFd = -1;
            return;
        }

        char buffer[bufferSize];
        while (running) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(inotifyFd, &fds);

            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            int selectResult = select(inotifyFd + 1, &fds, nullptr, nullptr, &timeout);
            
            if (selectResult > 0 && FD_ISSET(inotifyFd, &fds)) {
                ssize_t length = read(inotifyFd, buffer, bufferSize);
                if (length < 0) {
                    break;
                }

                size_t i = 0;
                while (i < static_cast<size_t>(length)) {
                    struct inotify_event* event = reinterpret_cast<struct inotify_event*>(&buffer[i]);
                    if (event->len) {
                        std::string fileName(event->name);
                        std::string fullPath = watchPath + "/" + fileName;
                        
                        if (callback) {
                            callback(fullPath, event->mask);
                        }
                    }
                    i += eventSize + event->len;
                }
            }
        }
    }
    #endif
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        
        watcher.setCallback([](const std::string& path, uint32_t action) {
            std::cout << "File system event detected:" << std::endl;
            std::cout << "  Path: " << path << std::endl;
            std::cout << "  Action: 0x" << std::hex << action << std::dec << std::endl;
            
            #ifdef _WIN32
                switch (action) {
                    case FILE_ACTION_ADDED: std::cout << "  Type: File Added" << std::endl; break;
                    case FILE_ACTION_REMOVED: std::cout << "  Type: File Removed" << std::endl; break;
                    case FILE_ACTION_MODIFIED: std::cout << "  Type: File Modified" << std::endl; break;
                    case FILE_ACTION_RENAMED_OLD_NAME: std::cout << "  Type: File Renamed (Old)" << std::endl; break;
                    case FILE_ACTION_RENAMED_NEW_NAME: std::cout << "  Type: File Renamed (New)" << std::endl; break;
                }
            #else
                if (action & IN_CREATE) std::cout << "  Type: File Created" << std::endl;
                if (action & IN_DELETE) std::cout << "  Type: File Deleted" << std::endl;
                if (action & IN_MODIFY) std::cout << "  Type: File Modified" << std::endl;
                if (action & IN_MOVED_FROM) std::cout << "  Type: File Moved From" << std::endl;
                if (action & IN_MOVED_TO) std::cout << "  Type: File Moved To" << std::endl;
            #endif
            
            std::cout << std::endl;
        });
        
        watcher.start();
        std::cout << "Watching current directory for file system changes..." << std::endl;
        std::cout << "Press Enter to stop watching..." << std::endl;
        
        std::cin.get();
        watcher.stop();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}