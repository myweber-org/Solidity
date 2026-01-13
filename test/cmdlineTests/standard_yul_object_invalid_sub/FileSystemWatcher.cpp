
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

class FileSystemWatcher {
private:
    std::string path_to_watch;
    std::atomic<bool> running{false};
    std::thread watch_thread;

    #ifdef _WIN32
        HANDLE directory_handle = INVALID_HANDLE_VALUE;
        HANDLE completion_port = nullptr;
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
        char buffer[1024];
        DWORD bytes_returned;
        OVERLAPPED overlapped{};
        FILE_NOTIFY_INFORMATION* notify_info;

        while (running) {
            if (ReadDirectoryChangesW(directory_handle,
                                     buffer,
                                     sizeof(buffer),
                                     TRUE,
                                     FILE_NOTIFY_CHANGE_FILE_NAME |
                                     FILE_NOTIFY_CHANGE_DIR_NAME |
                                     FILE_NOTIFY_CHANGE_ATTRIBUTES |
                                     FILE_NOTIFY_CHANGE_SIZE |
                                     FILE_NOTIFY_CHANGE_LAST_WRITE,
                                     &bytes_returned,
                                     &overlapped,
                                     nullptr)) {
                if (GetQueuedCompletionStatus(completion_port,
                                             &bytes_returned,
                                             (PULONG_PTR)&directory_handle,
                                             (LPOVERLAPPED*)&overlapped,
                                             INFINITE)) {
                    if (bytes_returned == 0) continue;

                    notify_info = (FILE_NOTIFY_INFORMATION*)buffer;
                    while (true) {
                        std::wstring filename(notify_info->FileName,
                                             notify_info->FileNameLength / sizeof(WCHAR));
                        std::string action_str;
                        switch (notify_info->Action) {
                            case FILE_ACTION_ADDED: action_str = "ADDED"; break;
                            case FILE_ACTION_REMOVED: action_str = "REMOVED"; break;
                            case FILE_ACTION_MODIFIED: action_str = "MODIFIED"; break;
                            case FILE_ACTION_RENAMED_OLD_NAME: action_str = "RENAMED_OLD"; break;
                            case FILE_ACTION_RENAMED_NEW_NAME: action_str = "RENAMED_NEW"; break;
                            default: action_str = "UNKNOWN";
                        }
                        std::cout << "[" << action_str << "] "
                                  << std::string(filename.begin(), filename.end())
                                  << std::endl;

                        if (notify_info->NextEntryOffset == 0) break;
                        notify_info = (FILE_NOTIFY_INFORMATION*)((BYTE*)notify_info +
                                                                 notify_info->NextEntryOffset);
                    }
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void watch_loop_linux() {
        char buffer[1024];
        while (running) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(inotify_fd, &fds);
            struct timeval timeout = {1, 0};

            int ready = select(inotify_fd + 1, &fds, nullptr, nullptr, &timeout);
            if (ready > 0 && FD_ISSET(inotify_fd, &fds)) {
                ssize_t length = read(inotify_fd, buffer, sizeof(buffer));
                if (length < 0) continue;

                ssize_t i = 0;
                while (i < length) {
                    struct inotify_event* event = (struct inotify_event*)&buffer[i];
                    if (event->len) {
                        std::string action_str;
                        if (event->mask & IN_CREATE) action_str = "CREATED";
                        else if (event->mask & IN_DELETE) action_str = "DELETED";
                        else if (event->mask & IN_MODIFY) action_str = "MODIFIED";
                        else if (event->mask & IN_MOVED_FROM) action_str = "MOVED_FROM";
                        else if (event->mask & IN_MOVED_TO) action_str = "MOVED_TO";
                        else action_str = "UNKNOWN";

                        std::cout << "[" << action_str << "] " << event->name << std::endl;
                    }
                    i += sizeof(struct inotify_event) + event->len;
                }
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
            directory_handle = CreateFileA(
                path_to_watch.c_str(),
                FILE_LIST_DIRECTORY,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                nullptr,
                OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                nullptr
            );

            if (directory_handle == INVALID_HANDLE_VALUE) {
                std::cerr << "Failed to open directory" << std::endl;
                return false;
            }

            completion_port = CreateIoCompletionPort(directory_handle,
                                                    nullptr,
                                                    0,
                                                    0);
            if (!completion_port) {
                CloseHandle(directory_handle);
                return false;
            }
        #else
            inotify_fd = inotify_init();
            if (inotify_fd < 0) {
                std::cerr << "Failed to initialize inotify" << std::endl;
                return false;
            }

            watch_descriptor = inotify_add_watch(inotify_fd,
                                                path_to_watch.c_str(),
                                                IN_CREATE | IN_DELETE |
                                                IN_MODIFY | IN_MOVE);
            if (watch_descriptor < 0) {
                close(inotify_fd);
                std::cerr << "Failed to add watch" << std::endl;
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
            if (completion_port) CloseHandle(completion_port);
            if (directory_handle != INVALID_HANDLE_VALUE) {
                CloseHandle(directory_handle);
            }
        #else
            if (watch_descriptor >= 0) {
                inotify_rm_watch(inotify_fd, watch_descriptor);
            }
            if (inotify_fd >= 0) {
                close(inotify_fd);
            }
        #endif
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        std::cout << "Watching directory: " << argv[1] << std::endl;
        std::cout << "Press Enter to stop..." << std::endl;

        if (!watcher.start()) {
            std::cerr << "Failed to start watcher" << std::endl;
            return 1;
        }

        std::cin.get();
        watcher.stop();
        std::cout << "Watcher stopped" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}