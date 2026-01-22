#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <filesystem>
#include <atomic>
#include <vector>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/inotify.h>
    #include <unistd.h>
    #include <limits.h>
#endif

class FileSystemWatcher {
private:
    std::string path;
    std::atomic<bool> running;
    std::thread watchThread;

#ifdef _WIN32
    HANDLE directoryHandle;
    OVERLAPPED overlapped;
    char buffer[1024];
#else
    int inotifyFd;
    int watchDescriptor;
#endif

    void watchLoop() {
#ifdef _WIN32
        while (running) {
            DWORD bytesReturned;
            if (ReadDirectoryChangesW(
                directoryHandle,
                buffer,
                sizeof(buffer),
                TRUE,
                FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE,
                &bytesReturned,
                &overlapped,
                NULL)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
#else
        char eventBuffer[1024];
        while (running) {
            int length = read(inotifyFd, eventBuffer, sizeof(eventBuffer));
            if (length < 0) {
                break;
            }
            int i = 0;
            while (i < length) {
                struct inotify_event* event = (struct inotify_event*)&eventBuffer[i];
                if (event->len) {
                    if (event->mask & IN_CREATE) {
                        std::cout << "File created: " << event->name << std::endl;
                    }
                    if (event->mask & IN_DELETE) {
                        std::cout << "File deleted: " << event->name << std::endl;
                    }
                    if (event->mask & IN_MODIFY) {
                        std::cout << "File modified: " << event->name << std::endl;
                    }
                }
                i += sizeof(struct inotify_event) + event->len;
            }
        }
#endif
    }

public:
    FileSystemWatcher(const std::string& watchPath) : path(watchPath), running(false) {
#ifdef _WIN32
        directoryHandle = CreateFileA(
            path.c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            NULL);
        memset(&overlapped, 0, sizeof(overlapped));
#else
        inotifyFd = inotify_init();
        if (inotifyFd < 0) {
            std::cerr << "Error initializing inotify" << std::endl;
            return;
        }
        watchDescriptor = inotify_add_watch(inotifyFd, path.c_str(), IN_CREATE | IN_DELETE | IN_MODIFY);
        if (watchDescriptor < 0) {
            std::cerr << "Error adding watch for path: " << path << std::endl;
            close(inotifyFd);
            return;
        }
#endif
    }

    ~FileSystemWatcher() {
        stop();
#ifdef _WIN32
        if (directoryHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(directoryHandle);
        }
#else
        if (inotifyFd >= 0) {
            close(inotifyFd);
        }
#endif
    }

    void start() {
        if (running) return;
        running = true;
        watchThread = std::thread(&FileSystemWatcher::watchLoop, this);
        std::cout << "Started watching: " << path << std::endl;
    }

    void stop() {
        if (!running) return;
        running = false;
        if (watchThread.joinable()) {
            watchThread.join();
        }
        std::cout << "Stopped watching: " << path << std::endl;
    }

    bool isRunning() const {
        return running;
    }
};

int main() {
    std::string currentPath = std::filesystem::current_path().string();
    FileSystemWatcher watcher(currentPath);
    watcher.start();
    std::this_thread::sleep_for(std::chrono::seconds(30));
    watcher.stop();
    return 0;
}