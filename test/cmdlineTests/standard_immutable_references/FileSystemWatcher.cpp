#include <iostream>
#include <sys/inotify.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>
#include <map>

class FileSystemWatcher {
private:
    int inotifyFd;
    std::map<int, std::string> watchDescriptors;

public:
    FileSystemWatcher() : inotifyFd(-1) {
        inotifyFd = inotify_init();
        if (inotifyFd < 0) {
            std::cerr << "Failed to initialize inotify" << std::endl;
        }
    }

    ~FileSystemWatcher() {
        for (auto& pair : watchDescriptors) {
            inotify_rm_watch(inotifyFd, pair.first);
        }
        if (inotifyFd >= 0) {
            close(inotifyFd);
        }
    }

    bool addWatch(const std::string& path, uint32_t mask) {
        int wd = inotify_add_watch(inotifyFd, path.c_str(), mask);
        if (wd < 0) {
            std::cerr << "Failed to add watch for " << path << std::endl;
            return false;
        }
        watchDescriptors[wd] = path;
        std::cout << "Watching: " << path << " (WD: " << wd << ")" << std::endl;
        return true;
    }

    void removeWatch(int wd) {
        if (inotify_rm_watch(inotifyFd, wd) == 0) {
            watchDescriptors.erase(wd);
            std::cout << "Removed watch descriptor: " << wd << std::endl;
        }
    }

    void startMonitoring() {
        constexpr size_t EVENT_SIZE = sizeof(struct inotify_event);
        constexpr size_t BUF_LEN = 1024 * (EVENT_SIZE + 16);
        char buffer[BUF_LEN];

        std::cout << "Starting filesystem monitoring..." << std::endl;

        while (true) {
            ssize_t length = read(inotifyFd, buffer, BUF_LEN);
            if (length < 0) {
                std::cerr << "Read error" << std::endl;
                break;
            }

            ssize_t i = 0;
            while (i < length) {
                struct inotify_event* event = reinterpret_cast<struct inotify_event*>(&buffer[i]);
                if (event->len) {
                    std::string path = watchDescriptors[event->wd];
                    std::cout << "Event detected on: " << path;
                    if (event->mask & IN_CREATE) std::cout << " - FILE CREATED";
                    if (event->mask & IN_DELETE) std::cout << " - FILE DELETED";
                    if (event->mask & IN_MODIFY) std::cout << " - FILE MODIFIED";
                    if (event->mask & IN_MOVED_FROM) std::cout << " - FILE MOVED FROM";
                    if (event->mask & IN_MOVED_TO) std::cout << " - FILE MOVED TO";
                    std::cout << std::endl;
                }
                i += EVENT_SIZE + event->len;
            }
        }
    }
};

int main() {
    FileSystemWatcher watcher;
    watcher.addWatch(".", IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO);
    watcher.startMonitoring();
    return 0;
}