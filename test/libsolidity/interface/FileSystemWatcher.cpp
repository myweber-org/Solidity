#include <iostream>
#include <sys/inotify.h>
#include <unistd.h>
#include <cstring>
#include <limits.h>

class FileSystemWatcher {
private:
    int inotifyFd;
    int watchDescriptor;
    static constexpr size_t EVENT_SIZE = sizeof(struct inotify_event);
    static constexpr size_t BUF_LEN = 1024 * (EVENT_SIZE + NAME_MAX + 1);

public:
    FileSystemWatcher() : inotifyFd(-1), watchDescriptor(-1) {
        inotifyFd = inotify_init();
        if (inotifyFd < 0) {
            std::cerr << "Failed to initialize inotify" << std::endl;
        }
    }

    bool addWatch(const std::string& path, uint32_t mask) {
        if (inotifyFd < 0) return false;
        
        watchDescriptor = inotify_add_watch(inotifyFd, path.c_str(), mask);
        if (watchDescriptor < 0) {
            std::cerr << "Failed to add watch for: " << path << std::endl;
            return false;
        }
        
        std::cout << "Watching: " << path << std::endl;
        return true;
    }

    void startMonitoring() {
        if (inotifyFd < 0 || watchDescriptor < 0) {
            std::cerr << "Watcher not properly initialized" << std::endl;
            return;
        }

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
                struct inotify_event* event = 
                    reinterpret_cast<struct inotify_event*>(&buffer[i]);
                
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
                    if (event->mask & IN_MOVED_FROM) {
                        std::cout << "File moved from: " << event->name << std::endl;
                    }
                    if (event->mask & IN_MOVED_TO) {
                        std::cout << "File moved to: " << event->name << std::endl;
                    }
                }
                i += EVENT_SIZE + event->len;
            }
        }
    }

    ~FileSystemWatcher() {
        if (watchDescriptor >= 0) {
            inotify_rm_watch(inotifyFd, watchDescriptor);
        }
        if (inotifyFd >= 0) {
            close(inotifyFd);
        }
    }
};

int main() {
    FileSystemWatcher watcher;
    
    if (watcher.addWatch(".", IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVE)) {
        watcher.startMonitoring();
    }
    
    return 0;
}