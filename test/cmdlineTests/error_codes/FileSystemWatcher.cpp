
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    SimpleFileWatcher(const fs::path& path, std::chrono::milliseconds interval)
        : watch_path(path), poll_interval(interval), running(false) {
        if (!fs::exists(watch_path)) {
            throw std::runtime_error("Path does not exist: " + watch_path.string());
        }
        build_snapshot();
    }

    void start() {
        running = true;
        watcher_thread = std::thread(&SimpleFileWatcher::watch_loop, this);
    }

    void stop() {
        running = false;
        if (watcher_thread.joinable()) {
            watcher_thread.join();
        }
    }

    ~SimpleFileWatcher() {
        stop();
    }

private:
    void build_snapshot() {
        file_snapshot.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                auto last_write = fs::last_write_time(entry);
                file_snapshot[entry.path()] = last_write;
            }
        }
    }

    void watch_loop() {
        while (running) {
            std::this_thread::sleep_for(poll_interval);
            check_for_changes();
        }
    }

    void check_for_changes() {
        std::unordered_map<fs::path, fs::file_time_type> current_snapshot;
        
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                auto last_write = fs::last_write_time(entry);
                current_snapshot[entry.path()] = last_write;
                
                auto it = file_snapshot.find(entry.path());
                if (it == file_snapshot.end()) {
                    std::cout << "[CREATED] " << entry.path() << std::endl;
                } else if (it->second != last_write) {
                    std::cout << "[MODIFIED] " << entry.path() << std::endl;
                }
            }
        }
        
        for (const auto& [path, _] : file_snapshot) {
            if (current_snapshot.find(path) == current_snapshot.end()) {
                std::cout << "[DELETED] " << path << std::endl;
            }
        }
        
        file_snapshot = std::move(current_snapshot);
    }

    fs::path watch_path;
    std::chrono::milliseconds poll_interval;
    std::unordered_map<fs::path, fs::file_time_type> file_snapshot;
    std::thread watcher_thread;
    std::atomic<bool> running;
};

int main() {
    try {
        SimpleFileWatcher watcher(fs::current_path(), std::chrono::seconds(2));
        watcher.start();
        
        std::cout << "Watching directory: " << fs::current_path() << std::endl;
        std::cout << "Press Enter to stop..." << std::endl;
        std::cin.get();
        
        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}#include <iostream>
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

        char buffer[4096];
        DWORD bytesReturned;

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
                NULL,
                NULL)) {

                FILE_NOTIFY_INFORMATION* notifyInfo = (FILE_NOTIFY_INFORMATION*)buffer;
                while (notifyInfo != NULL) {
                    handleEvent(notifyInfo);
                    if (notifyInfo->NextEntryOffset == 0) break;
                    notifyInfo = (FILE_NOTIFY_INFORMATION*)((BYTE*)notifyInfo + notifyInfo->NextEntryOffset);
                }
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
            IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);

        if (watchDescriptor < 0) {
            std::cerr << "Failed to add watch for: " << watchPath << std::endl;
            close(inotifyFd);
            return;
        }

        char buffer[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
        const struct inotify_event* event;

        while (running) {
            ssize_t length = read(inotifyFd, buffer, sizeof(buffer));
            if (length > 0) {
                for (char* ptr = buffer; ptr < buffer + length; ptr += sizeof(struct inotify_event) + event->len) {
                    event = (const struct inotify_event*)ptr;
                    handleEvent(event);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        inotify_rm_watch(inotifyFd, watchDescriptor);
        close(inotifyFd);
#endif
    }

#ifdef _WIN32
    void handleEvent(FILE_NOTIFY_INFORMATION* info) {
        std::wstring wideFilename(info->FileName, info->FileNameLength / sizeof(WCHAR));
        std::string filename(wideFilename.begin(), wideFilename.end());
        std::string fullPath = watchPath + "\\" + filename;

        switch (info->Action) {
        case FILE_ACTION_ADDED:
            std::cout << "File created: " << fullPath << std::endl;
            break;
        case FILE_ACTION_REMOVED:
            std::cout << "File deleted: " << fullPath << std::endl;
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
    void handleEvent(const struct inotify_event* event) {
        std::string filename = event->name;
        std::string fullPath = watchPath + "/" + filename;

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
#endif
};

int main() {
    std::filesystem::path currentPath = std::filesystem::current_path();
    FileSystemWatcher watcher(currentPath.string());

    std::cout << "Watching directory: " << currentPath.string() << std::endl;
    std::cout << "Press Enter to stop watching..." << std::endl;

    if (watcher.start()) {
        std::cin.get();
        watcher.stop();
    }

    return 0;
}