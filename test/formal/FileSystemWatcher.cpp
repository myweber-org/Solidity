#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path path_to_watch;
    std::unordered_set<std::string> current_files;
    bool running = false;

    void populate_file_set() {
        current_files.clear();
        for (const auto& entry : fs::directory_iterator(path_to_watch)) {
            current_files.insert(entry.path().filename().string());
        }
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        populate_file_set();
    }

    void start(int interval_seconds = 1) {
        running = true;
        std::cout << "Starting to watch: " << path_to_watch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            auto old_files = current_files;
            populate_file_set();

            for (const auto& fname : current_files) {
                if (old_files.find(fname) == old_files.end()) {
                    std::cout << "File added: " << fname << std::endl;
                }
            }

            for (const auto& fname : old_files) {
                if (current_files.find(fname) == current_files.end()) {
                    std::cout << "File removed: " << fname << std::endl;
                }
            }
        }
    }

    void stop() {
        running = false;
        std::cout << "Stopped watching." << std::endl;
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start(2);
        std::this_thread::sleep_for(std::chrono::seconds(10));
        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
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

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const std::string& path) : watchPath(path), running(false) {
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
            }
        #else
            if (inotifyFd >= 0) {
                close(inotifyFd);
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
                FILE_NOTIFY_INFORMATION* notifyInfo = (FILE_NOTIFY_INFORMATION*)buffer;
                do {
                    std::wstring wideFileName(notifyInfo->FileName, notifyInfo->FileNameLength / sizeof(WCHAR));
                    std::string fileName(wideFileName.begin(), wideFileName.end());
                    std::string fullPath = watchPath + "\\" + fileName;

                    if (eventCallback) {
                        eventCallback(fullPath, notifyInfo->Action);
                    }

                    if (notifyInfo->NextEntryOffset == 0) break;
                    notifyInfo = (FILE_NOTIFY_INFORMATION*)((BYTE*)notifyInfo + notifyInfo->NextEntryOffset);
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

        watchDescriptor = inotify_add_watch(inotifyFd, watchPath.c_str(),
                                           IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO);

        if (watchDescriptor < 0) {
            std::cerr << "Failed to add watch for directory" << std::endl;
            close(inotifyFd);
            return;
        }

        char buffer[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
        const struct inotify_event* event;

        while (running) {
            ssize_t length = read(inotifyFd, buffer, sizeof(buffer));
            if (length < 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            for (char* ptr = buffer; ptr < buffer + length; ptr += sizeof(struct inotify_event) + event->len) {
                event = (const struct inotify_event*)ptr;
                
                if (event->len) {
                    std::string fileName(event->name);
                    std::string fullPath = watchPath + "/" + fileName;

                    if (eventCallback) {
                        eventCallback(fullPath, event->mask);
                    }
                }
            }
        }
    }
    #endif
};

void exampleCallback(const std::string& path, uint32_t eventType) {
    std::string eventStr;
    #ifdef _WIN32
        switch (eventType) {
            case FILE_ACTION_ADDED: eventStr = "ADDED"; break;
            case FILE_ACTION_REMOVED: eventStr = "REMOVED"; break;
            case FILE_ACTION_MODIFIED: eventStr = "MODIFIED"; break;
            case FILE_ACTION_RENAMED_OLD_NAME: eventStr = "RENAMED_OLD"; break;
            case FILE_ACTION_RENAMED_NEW_NAME: eventStr = "RENAMED_NEW"; break;
            default: eventStr = "UNKNOWN"; break;
        }
    #else
        if (eventType & IN_CREATE) eventStr = "CREATED";
        else if (eventType & IN_DELETE) eventStr = "DELETED";
        else if (eventType & IN_MODIFY) eventStr = "MODIFIED";
        else if (eventType & IN_MOVED_FROM) eventStr = "MOVED_FROM";
        else if (eventType & IN_MOVED_TO) eventStr = "MOVED_TO";
        else eventStr = "UNKNOWN";
    #endif
    
    std::cout << "Event: " << eventStr << " | Path: " << path << std::endl;
}

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.setEventCallback(exampleCallback);
        watcher.start();

        std::cout << "Watching current directory. Press Enter to stop..." << std::endl;
        std::cin.get();

        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}