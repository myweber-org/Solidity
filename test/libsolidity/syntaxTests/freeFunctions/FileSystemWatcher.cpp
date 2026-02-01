
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    SimpleFileWatcher(const fs::path& path) : watch_path(path), running(false) {
        if (!fs::exists(path) || !fs::is_directory(path)) {
            throw std::runtime_error("Path does not exist or is not a directory");
        }
        updateSnapshot();
    }

    void start(int interval_seconds = 1) {
        running = true;
        watcher_thread = std::thread(&SimpleFileWatcher::watchLoop, this, interval_seconds);
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
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_snapshot;
    std::thread watcher_thread;
    bool running;

    void updateSnapshot() {
        file_snapshot.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_snapshot[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void watchLoop(int interval_seconds) {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            checkForChanges();
        }
    }

    void checkForChanges() {
        std::unordered_map<std::string, fs::file_time_type> current_state;

        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                std::string path_str = entry.path().string();
                current_state[path_str] = fs::last_write_time(entry.path());
            }
        }

        for (const auto& [path, old_time] : file_snapshot) {
            if (current_state.find(path) == current_state.end()) {
                std::cout << "File deleted: " << path << std::endl;
            } else if (current_state[path] != old_time) {
                std::cout << "File modified: " << path << std::endl;
            }
        }

        for (const auto& [path, new_time] : current_state) {
            if (file_snapshot.find(path) == file_snapshot.end()) {
                std::cout << "File created: " << path << std::endl;
            }
        }

        file_snapshot = std::move(current_state);
    }
};

int main() {
    try {
        SimpleFileWatcher watcher(".");
        watcher.start(2);

        std::cout << "Watching current directory. Press Enter to stop..." << std::endl;
        std::cin.get();

        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path directory_to_watch;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps;
    bool running = false;

    void populate_file_map() {
        file_timestamps.clear();
        for (const auto& entry : fs::directory_iterator(directory_to_watch)) {
            if (fs::is_regular_file(entry.status())) {
                file_timestamps[entry.path().filename().string()] = fs::last_write_time(entry);
            }
        }
    }

public:
    explicit FileSystemWatcher(const std::string& dir_path) : directory_to_watch(dir_path) {
        if (!fs::exists(directory_to_watch) || !fs::is_directory(directory_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        populate_file_map();
        std::cout << "Watching directory: " << directory_to_watch << std::endl;
    }

    void start(int interval_seconds = 1) {
        running = true;
        std::cout << "Starting watch loop (interval: " << interval_seconds << "s). Press Ctrl+C to stop." << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            for (const auto& entry : fs::directory_iterator(directory_to_watch)) {
                if (!fs::is_regular_file(entry.status())) {
                    continue;
                }

                std::string filename = entry.path().filename().string();
                auto current_write_time = fs::last_write_time(entry);

                if (file_timestamps.find(filename) == file_timestamps.end()) {
                    std::cout << "[NEW] File added: " << filename << std::endl;
                    file_timestamps[filename] = current_write_time;
                } else if (file_timestamps[filename] != current_write_time) {
                    std::cout << "[MODIFIED] File changed: " << filename << std::endl;
                    file_timestamps[filename] = current_write_time;
                }
            }

            for (auto it = file_timestamps.begin(); it != file_timestamps.end(); ) {
                fs::path file_path = directory_to_watch / it->first;
                if (!fs::exists(file_path)) {
                    std::cout << "[DELETED] File removed: " << it->first << std::endl;
                    it = file_timestamps.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    void stop() {
        running = false;
        std::cout << "Stopping file system watcher." << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start(2);
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
    void watchLoop() {
#ifdef _WIN32
        watchWindows();
#else
        watchLinux();
#endif
    }

#ifdef _WIN32
    void watchWindows() {
        HANDLE hDir = CreateFileA(
            watchPath.c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            NULL
        );

        if (hDir == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to open directory: " << watchPath << std::endl;
            return;
        }

        char buffer[4096];
        DWORD bytesReturned;
        OVERLAPPED overlapped = {0};
        overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        while (running) {
            if (ReadDirectoryChangesW(
                hDir,
                buffer,
                sizeof(buffer),
                TRUE,
                FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
                FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
                &bytesReturned,
                &overlapped,
                NULL)) {
                
                WaitForSingleObject(overlapped.hEvent, INFINITE);
                
                if (bytesReturned > 0) {
                    FILE_NOTIFY_INFORMATION* fni = (FILE_NOTIFY_INFORMATION*)buffer;
                    do {
                        std::wstring fileName(fni->FileName, fni->FileNameLength / sizeof(WCHAR));
                        handleFileEvent(fileName, fni->Action);
                        
                        if (fni->NextEntryOffset == 0) break;
                        fni = (FILE_NOTIFY_INFORMATION*)((BYTE*)fni + fni->NextEntryOffset);
                    } while (true);
                }
            }
            
            ResetEvent(overlapped.hEvent);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        CloseHandle(overlapped.hEvent);
        CloseHandle(hDir);
    }
#else
    void watchLinux() {
        int fd = inotify_init();
        if (fd < 0) {
            std::cerr << "Failed to initialize inotify" << std::endl;
            return;
        }

        int wd = inotify_add_watch(fd, watchPath.c_str(),
                                  IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO);
        
        if (wd < 0) {
            std::cerr << "Failed to add watch for: " << watchPath << std::endl;
            close(fd);
            return;
        }

        char buffer[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
        
        while (running) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(fd, &fds);
            
            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
            
            int ret = select(fd + 1, &fds, NULL, NULL, &timeout);
            
            if (ret > 0 && FD_ISSET(fd, &fds)) {
                ssize_t len = read(fd, buffer, sizeof(buffer));
                if (len > 0) {
                    const struct inotify_event* event;
                    for (char* ptr = buffer; ptr < buffer + len; 
                         ptr += sizeof(struct inotify_event) + event->len) {
                        event = (const struct inotify_event*)ptr;
                        handleInotifyEvent(event);
                    }
                }
            }
        }

        inotify_rm_watch(fd, wd);
        close(fd);
    }
#endif

    void handleFileEvent(const std::wstring& fileName, DWORD action) {
        std::string actionStr;
        switch (action) {
            case FILE_ACTION_ADDED: actionStr = "ADDED"; break;
            case FILE_ACTION_REMOVED: actionStr = "REMOVED"; break;
            case FILE_ACTION_MODIFIED: actionStr = "MODIFIED"; break;
            case FILE_ACTION_RENAMED_OLD_NAME: actionStr = "RENAMED_OLD"; break;
            case FILE_ACTION_RENAMED_NEW_NAME: actionStr = "RENAMED_NEW"; break;
            default: actionStr = "UNKNOWN"; break;
        }
        
        std::wcout << L"File " << fileName << L" - Action: " << actionStr.c_str() << std::endl;
    }

    void handleInotifyEvent(const struct inotify_event* event) {
        if (event->len > 0) {
            std::string fileName(event->name);
            std::string maskStr;
            
            if (event->mask & IN_CREATE) maskStr += "CREATE ";
            if (event->mask & IN_DELETE) maskStr += "DELETE ";
            if (event->mask & IN_MODIFY) maskStr += "MODIFY ";
            if (event->mask & IN_MOVED_FROM) maskStr += "MOVED_FROM ";
            if (event->mask & IN_MOVED_TO) maskStr += "MOVED_TO ";
            
            std::cout << "File " << fileName << " - Events: " << maskStr << std::endl;
        }
    }

    std::string watchPath;
    std::atomic<bool> running;
    std::thread watcherThread;
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