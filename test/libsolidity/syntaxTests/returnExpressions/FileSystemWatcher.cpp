
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <filesystem>
#include <atomic>
#include <functional>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/inotify.h>
    #include <unistd.h>
    #include <errno.h>
#endif

class FileSystemWatcher {
public:
    using Callback = std::function<void(const std::string&, uint32_t)>;

    enum EventType {
        CREATED = 0x01,
        DELETED = 0x02,
        MODIFIED = 0x04,
        RENAMED = 0x08
    };

    FileSystemWatcher() : running_(false) {}

    ~FileSystemWatcher() {
        stop();
    }

    bool addWatch(const std::string& path, uint32_t events, Callback callback) {
        std::filesystem::path fsPath(path);
        if (!std::filesystem::exists(fsPath)) {
            std::cerr << "Path does not exist: " << path << std::endl;
            return false;
        }

        WatchInfo info;
        info.path = std::filesystem::absolute(fsPath).string();
        info.events = events;
        info.callback = callback;
        watches_.push_back(info);

        return true;
    }

    bool start() {
        if (running_) return false;

        running_ = true;
        watchThread_ = std::thread(&FileSystemWatcher::watchLoop, this);
        return true;
    }

    void stop() {
        running_ = false;
        if (watchThread_.joinable()) {
            watchThread_.join();
        }
    }

private:
    struct WatchInfo {
        std::string path;
        uint32_t events;
        Callback callback;
    };

    std::vector<WatchInfo> watches_;
    std::thread watchThread_;
    std::atomic<bool> running_;

    void watchLoop() {
        #ifdef _WIN32
            watchLoopWindows();
        #else
            watchLoopLinux();
        #endif
    }

    #ifdef _WIN32
    void watchLoopWindows() {
        std::vector<HANDLE> dirHandles;
        std::vector<OVERLAPPED> overlappedVec;
        std::vector<char> buffers;

        for (const auto& watch : watches_) {
            HANDLE dirHandle = CreateFileA(
                watch.path.c_str(),
                FILE_LIST_DIRECTORY,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                NULL,
                OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                NULL
            );

            if (dirHandle == INVALID_HANDLE_VALUE) {
                continue;
            }

            dirHandles.push_back(dirHandle);
            overlappedVec.emplace_back();
            memset(&overlappedVec.back(), 0, sizeof(OVERLAPPED));
            buffers.resize(4096);
        }

        while (running_) {
            for (size_t i = 0; i < dirHandles.size(); ++i) {
                DWORD bytesReturned;
                if (ReadDirectoryChangesW(
                    dirHandles[i],
                    buffers.data(),
                    static_cast<DWORD>(buffers.size()),
                    TRUE,
                    FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                    FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
                    FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
                    &bytesReturned,
                    &overlappedVec[i],
                    NULL
                )) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
        }

        for (auto handle : dirHandles) {
            CloseHandle(handle);
        }
    }
    #else
    void watchLoopLinux() {
        int inotifyFd = inotify_init1(IN_NONBLOCK);
        if (inotifyFd < 0) {
            std::cerr << "Failed to initialize inotify" << std::endl;
            return;
        }

        std::vector<int> watchDescriptors;
        for (const auto& watch : watches_) {
            uint32_t mask = 0;
            if (watch.events & CREATED) mask |= IN_CREATE;
            if (watch.events & DELETED) mask |= IN_DELETE;
            if (watch.events & MODIFIED) mask |= IN_MODIFY;
            if (watch.events & RENAMED) mask |= IN_MOVE;

            int wd = inotify_add_watch(inotifyFd, watch.path.c_str(), mask);
            if (wd >= 0) {
                watchDescriptors.push_back(wd);
            }
        }

        char buffer[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
        const struct inotify_event* event;

        while (running_) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(inotifyFd, &fds);

            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            int ready = select(inotifyFd + 1, &fds, NULL, NULL, &timeout);
            if (ready > 0 && FD_ISSET(inotifyFd, &fds)) {
                ssize_t len = read(inotifyFd, buffer, sizeof(buffer));
                if (len > 0) {
                    for (char* ptr = buffer; ptr < buffer + len; 
                         ptr += sizeof(struct inotify_event) + event->len) {
                        event = reinterpret_cast<const struct inotify_event*>(ptr);
                        
                        for (const auto& watch : watches_) {
                            uint32_t eventType = 0;
                            if (event->mask & IN_CREATE) eventType |= CREATED;
                            if (event->mask & IN_DELETE) eventType |= DELETED;
                            if (event->mask & IN_MODIFY) eventType |= MODIFIED;
                            if (event->mask & IN_MOVE) eventType |= RENAMED;

                            if (eventType & watch.events) {
                                watch.callback(event->name, eventType);
                            }
                        }
                    }
                }
            }
        }

        for (int wd : watchDescriptors) {
            inotify_rm_watch(inotifyFd, wd);
        }
        close(inotifyFd);
    }
    #endif
};

void exampleCallback(const std::string& filename, uint32_t events) {
    std::string eventStr;
    if (events & FileSystemWatcher::CREATED) eventStr += "CREATED ";
    if (events & FileSystemWatcher::DELETED) eventStr += "DELETED ";
    if (events & FileSystemWatcher::MODIFIED) eventStr += "MODIFIED ";
    if (events & FileSystemWatcher::RENAMED) eventStr += "RENAMED ";
    
    std::cout << "File: " << filename << " Events: " << eventStr << std::endl;
}

int main() {
    FileSystemWatcher watcher;
    
    watcher.addWatch(".", 
        FileSystemWatcher::CREATED | 
        FileSystemWatcher::DELETED | 
        FileSystemWatcher::MODIFIED,
        exampleCallback);
    
    std::cout << "Watching current directory. Press Enter to stop..." << std::endl;
    
    watcher.start();
    
    std::cin.get();
    
    watcher.stop();
    
    return 0;
}