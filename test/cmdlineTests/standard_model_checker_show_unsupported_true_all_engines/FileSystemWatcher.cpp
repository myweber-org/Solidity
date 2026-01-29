
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
    std::string path_to_watch;
    std::atomic<bool> running{false};
    std::thread watch_thread;

    #ifdef _WIN32
        HANDLE dir_handle = INVALID_HANDLE_VALUE;
        HANDLE completion_port = INVALID_HANDLE_VALUE;
    #else
        int inotify_fd = -1;
        int watch_descriptor = -1;
    #endif

    void watch_loop() {
        #ifdef _WIN32
            watch_loop_windows();
        #else
            watch_loop_linux();
        #endif
    }

    void watch_loop_windows() {
        FILE_NOTIFY_INFORMATION buffer[1024];
        DWORD bytes_returned;

        while (running) {
            if (ReadDirectoryChangesW(dir_handle,
                                     buffer,
                                     sizeof(buffer),
                                     TRUE,
                                     FILE_NOTIFY_CHANGE_FILE_NAME |
                                     FILE_NOTIFY_CHANGE_DIR_NAME |
                                     FILE_NOTIFY_CHANGE_ATTRIBUTES |
                                     FILE_NOTIFY_CHANGE_SIZE |
                                     FILE_NOTIFY_CHANGE_LAST_WRITE,
                                     &bytes_returned,
                                     NULL,
                                     NULL)) {

                FILE_NOTIFY_INFORMATION* info = buffer;
                do {
                    std::wstring filename_wide(info->FileName, info->FileNameLength / sizeof(WCHAR));
                    std::string filename(filename_wide.begin(), filename_wide.end());

                    std::string action;
                    switch (info->Action) {
                        case FILE_ACTION_ADDED: action = "ADDED"; break;
                        case FILE_ACTION_REMOVED: action = "REMOVED"; break;
                        case FILE_ACTION_MODIFIED: action = "MODIFIED"; break;
                        case FILE_ACTION_RENAMED_OLD_NAME: action = "RENAMED_OLD"; break;
                        case FILE_ACTION_RENAMED_NEW_NAME: action = "RENAMED_NEW"; break;
                        default: action = "UNKNOWN"; break;
                    }

                    std::cout << "[" << action << "] " << filename << std::endl;

                    if (info->NextEntryOffset == 0) break;
                    info = (FILE_NOTIFY_INFORMATION*)((BYTE*)info + info->NextEntryOffset);
                } while (true);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void watch_loop_linux() {
        char buffer[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
        const struct inotify_event* event;

        while (running) {
            ssize_t len = read(inotify_fd, buffer, sizeof(buffer));
            if (len <= 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            for (char* ptr = buffer; ptr < buffer + len; ptr += sizeof(struct inotify_event) + event->len) {
                event = (const struct inotify_event*)ptr;

                std::string action;
                if (event->mask & IN_CREATE) action = "CREATED";
                else if (event->mask & IN_DELETE) action = "DELETED";
                else if (event->mask & IN_MODIFY) action = "MODIFIED";
                else if (event->mask & IN_MOVED_FROM) action = "MOVED_FROM";
                else if (event->mask & IN_MOVED_TO) action = "MOVED_TO";
                else action = "UNKNOWN";

                std::cout << "[" << action << "] " << event->name << std::endl;
            }
        }
    }

public:
    explicit FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!std::filesystem::exists(path)) {
            throw std::runtime_error("Path does not exist: " + path);
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

    bool start() {
        if (running) return false;

        #ifdef _WIN32
            dir_handle = CreateFileA(path_to_watch.c_str(),
                                    FILE_LIST_DIRECTORY,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                                    NULL);

            if (dir_handle == INVALID_HANDLE_VALUE) {
                std::cerr << "Failed to open directory for watching." << std::endl;
                return false;
            }
        #else
            inotify_fd = inotify_init();
            if (inotify_fd < 0) {
                std::cerr << "Failed to initialize inotify." << std::endl;
                return false;
            }

            watch_descriptor = inotify_add_watch(inotify_fd,
                                                path_to_watch.c_str(),
                                                IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVE);
            if (watch_descriptor < 0) {
                std::cerr << "Failed to add watch for directory." << std::endl;
                close(inotify_fd);
                return false;
            }
        #endif

        running = true;
        watch_thread = std::thread(&FileSystemWatcher::watch_loop, this);
        return true;
    }

    void stop() {
        if (!running) return;
        running = false;

        if (watch_thread.joinable()) {
            watch_thread.join();
        }

        #ifdef _WIN32
            if (dir_handle != INVALID_HANDLE_VALUE) {
                CloseHandle(dir_handle);
                dir_handle = INVALID_HANDLE_VALUE;
            }
        #else
            if (watch_descriptor >= 0) {
                inotify_rm_watch(inotify_fd, watch_descriptor);
                watch_descriptor = -1;
            }
            if (inotify_fd >= 0) {
                close(inotify_fd);
                inotify_fd = -1;
            }
        #endif
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        std::cout << "Watching directory: " << argv[1] << std::endl;
        std::cout << "Press Enter to stop..." << std::endl;

        if (!watcher.start()) {
            return 1;
        }

        std::cin.get();
        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}