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
private:
    std::string path_to_watch;
    std::atomic<bool> running{false};
    std::thread watch_thread;

    void watch_directory() {
#ifdef _WIN32
        HANDLE dir_handle = CreateFileA(
            path_to_watch.c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            NULL
        );

        if (dir_handle == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to open directory: " << path_to_watch << std::endl;
            return;
        }

        char buffer[1024];
        DWORD bytes_returned;
        OVERLAPPED overlapped = {0};
        overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        while (running) {
            if (ReadDirectoryChangesW(
                dir_handle,
                buffer,
                sizeof(buffer),
                TRUE,
                FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES |
                FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
                &bytes_returned,
                &overlapped,
                NULL
            )) {
                WaitForSingleObject(overlapped.hEvent, INFINITE);
                if (bytes_returned > 0) {
                    FILE_NOTIFY_INFORMATION* notify_info = (FILE_NOTIFY_INFORMATION*)buffer;
                    do {
                        std::wstring filename(notify_info->FileName, notify_info->FileNameLength / sizeof(WCHAR));
                        std::wcout << L"Change detected in: " << filename << std::endl;
                        if (notify_info->NextEntryOffset == 0) break;
                        notify_info = (FILE_NOTIFY_INFORMATION*)((BYTE*)notify_info + notify_info->NextEntryOffset);
                    } while (true);
                }
                ResetEvent(overlapped.hEvent);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        CloseHandle(overlapped.hEvent);
        CloseHandle(dir_handle);
#else
        int inotify_fd = inotify_init();
        if (inotify_fd < 0) {
            std::cerr << "Failed to initialize inotify" << std::endl;
            return;
        }

        int watch_desc = inotify_add_watch(inotify_fd, path_to_watch.c_str(),
                                          IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_TO | IN_MOVED_FROM);
        if (watch_desc < 0) {
            std::cerr << "Failed to add watch for: " << path_to_watch << std::endl;
            close(inotify_fd);
            return;
        }

        char buffer[1024];
        while (running) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(inotify_fd, &fds);

            struct timeval timeout = {1, 0};
            int ready = select(inotify_fd + 1, &fds, NULL, NULL, &timeout);

            if (ready > 0 && FD_ISSET(inotify_fd, &fds)) {
                int length = read(inotify_fd, buffer, sizeof(buffer));
                if (length < 0) continue;

                int i = 0;
                while (i < length) {
                    struct inotify_event* event = (struct inotify_event*)&buffer[i];
                    if (event->len) {
                        std::cout << "Change detected: " << event->name;
                        if (event->mask & IN_CREATE) std::cout << " (CREATED)";
                        if (event->mask & IN_DELETE) std::cout << " (DELETED)";
                        if (event->mask & IN_MODIFY) std::cout << " (MODIFIED)";
                        if (event->mask & IN_MOVED_FROM) std::cout << " (MOVED FROM)";
                        if (event->mask & IN_MOVED_TO) std::cout << " (MOVED TO)";
                        std::cout << std::endl;
                    }
                    i += sizeof(struct inotify_event) + event->len;
                }
            }
        }

        inotify_rm_watch(inotify_fd, watch_desc);
        close(inotify_fd);
#endif
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!std::filesystem::exists(path)) {
            throw std::runtime_error("Path does not exist: " + path);
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

    void start() {
        if (running) return;
        running = true;
        watch_thread = std::thread(&FileSystemWatcher::watch_directory, this);
        std::cout << "Started watching: " << path_to_watch << std::endl;
    }

    void stop() {
        if (!running) return;
        running = false;
        if (watch_thread.joinable()) {
            watch_thread.join();
        }
        std::cout << "Stopped watching: " << path_to_watch << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start();

        std::cout << "Press Enter to stop watching..." << std::endl;
        std::cin.get();

        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}