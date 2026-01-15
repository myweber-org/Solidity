#include <iostream>
#include <string>
#include <vector>
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
public:
    FileSystemWatcher(const std::string& path) : watchPath(path), running(false) {}

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

    std::vector<std::string> getChanges() {
        std::lock_guard<std::mutex> lock(changesMutex);
        std::vector<std::string> currentChanges;
        currentChanges.swap(detectedChanges);
        return currentChanges;
    }

private:
    std::string watchPath;
    std::atomic<bool> running;
    std::thread watcherThread;
    std::vector<std::string> detectedChanges;
    std::mutex changesMutex;

    #ifdef _WIN32
    HANDLE directoryHandle = INVALID_HANDLE_VALUE;
    #else
    int inotifyFd = -1;
    int watchDescriptor = -1;
    #endif

    void watchLoop() {
        #ifdef _WIN32
        setupWindowsWatcher();
        #else
        setupLinuxWatcher();
        #endif

        while (running) {
            #ifdef _WIN32
            pollWindowsEvents();
            #else
            pollLinuxEvents();
            #endif
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    #ifdef _WIN32
    void setupWindowsWatcher() {
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
            running = false;
        }
    }

    void pollWindowsEvents() {
        if (directoryHandle == INVALID_HANDLE_VALUE) return;

        char buffer[4096];
        DWORD bytesReturned;
        FILE_NOTIFY_INFORMATION* notifyInfo;

        if (ReadDirectoryChangesW(
            directoryHandle,
            buffer,
            sizeof(buffer),
            TRUE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
            FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
            FILE_NOTIFY_CHANGE_LAST_WRITE,
            &bytesReturned,
            NULL,
            NULL)) {

            notifyInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
            
            do {
                std::wstring wideFileName(notifyInfo->FileName, 
                                         notifyInfo->FileNameLength / sizeof(WCHAR));
                std::string fileName(wideFileName.begin(), wideFileName.end());
                
                std::lock_guard<std::mutex> lock(changesMutex);
                detectedChanges.push_back(fileName + " (event: " + 
                                         getEventName(notifyInfo->Action) + ")");

                if (notifyInfo->NextEntryOffset == 0) break;
                notifyInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                    reinterpret_cast<BYTE*>(notifyInfo) + notifyInfo->NextEntryOffset);
            } while (true);
        }
    }

    std::string getEventName(DWORD action) {
        switch (action) {
            case FILE_ACTION_ADDED: return "ADDED";
            case FILE_ACTION_REMOVED: return "REMOVED";
            case FILE_ACTION_MODIFIED: return "MODIFIED";
            case FILE_ACTION_RENAMED_OLD_NAME: return "RENAMED_OLD";
            case FILE_ACTION_RENAMED_NEW_NAME: return "RENAMED_NEW";
            default: return "UNKNOWN";
        }
    }
    #else
    void setupLinuxWatcher() {
        inotifyFd = inotify_init();
        if (inotifyFd < 0) {
            std::cerr << "Failed to initialize inotify" << std::endl;
            running = false;
            return;
        }

        watchDescriptor = inotify_add_watch(inotifyFd, watchPath.c_str(),
                                           IN_CREATE | IN_DELETE | IN_MODIFY | 
                                           IN_MOVED_FROM | IN_MOVED_TO);
        
        if (watchDescriptor < 0) {
            std::cerr << "Failed to add watch for directory" << std::endl;
            close(inotifyFd);
            inotifyFd = -1;
            running = false;
        }
    }

    void pollLinuxEvents() {
        if (inotifyFd < 0) return;

        char buffer[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
        ssize_t length = read(inotifyFd, buffer, sizeof(buffer));
        
        if (length <= 0) return;

        const struct inotify_event* event;
        for (char* ptr = buffer; ptr < buffer + length; 
             ptr += sizeof(struct inotify_event) + event->len) {
            
            event = reinterpret_cast<const struct inotify_event*>(ptr);
            
            if (event->len) {
                std::lock_guard<std::mutex> lock(changesMutex);
                std::string eventName = getInotifyEventName(event->mask);
                detectedChanges.push_back(std::string(event->name) + 
                                         " (event: " + eventName + ")");
            }
        }
    }

    std::string getInotifyEventName(uint32_t mask) {
        std::string result;
        
        if (mask & IN_CREATE) result += "CREATE|";
        if (mask & IN_DELETE) result += "DELETE|";
        if (mask & IN_MODIFY) result += "MODIFY|";
        if (mask & IN_MOVED_FROM) result += "MOVED_FROM|";
        if (mask & IN_MOVED_TO) result += "MOVED_TO|";
        
        if (!result.empty()) result.pop_back();
        return result.empty() ? "UNKNOWN" : result;
    }
    #endif
};

int main() {
    std::filesystem::path currentPath = std::filesystem::current_path();
    FileSystemWatcher watcher(currentPath.string());
    
    std::cout << "Watching directory: " << currentPath.string() << std::endl;
    std::cout << "Press Enter to stop watching..." << std::endl;
    
    watcher.start();
    
    auto lastCheck = std::chrono::steady_clock::now();
    while (true) {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastCheck).count() >= 2) {
            auto changes = watcher.getChanges();
            if (!changes.empty()) {
                std::cout << "\nDetected changes:" << std::endl;
                for (const auto& change : changes) {
                    std::cout << "  - " << change << std::endl;
                }
            }
            lastCheck = now;
        }
        
        if (std::cin.peek() != EOF) {
            std::string input;
            std::getline(std::cin, input);
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    watcher.stop();
    std::cout << "File system watcher stopped." << std::endl;
    
    return 0;
}