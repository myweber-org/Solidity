
#include <sys/inotify.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <limits>

class FileWatcher {
private:
    int inotifyFd;
    std::vector<int> watchDescriptors;

public:
    FileWatcher() : inotifyFd(-1) {
        inotifyFd = inotify_init();
        if (inotifyFd < 0) {
            std::cerr << "Failed to initialize inotify" << std::endl;
        }
    }

    ~FileWatcher() {
        for (int wd : watchDescriptors) {
            inotify_rm_watch(inotifyFd, wd);
        }
        if (inotifyFd >= 0) {
            close(inotifyFd);
        }
    }

    bool addWatch(const std::string& path, uint32_t mask) {
        if (inotifyFd < 0) return false;
        
        int wd = inotify_add_watch(inotifyFd, path.c_str(), mask);
        if (wd < 0) {
            std::cerr << "Failed to add watch for: " << path << std::endl;
            return false;
        }
        
        watchDescriptors.push_back(wd);
        std::cout << "Watching: " << path << std::endl;
        return true;
    }

    void processEvents() {
        if (inotifyFd < 0) return;

        const size_t eventSize = sizeof(inotify_event);
        const size_t bufferSize = 1024 * (eventSize + 16);
        char buffer[bufferSize];

        while (true) {
            ssize_t length = read(inotifyFd, buffer, bufferSize);
            if (length < 0) {
                std::cerr << "Error reading inotify events" << std::endl;
                break;
            }

            for (char* ptr = buffer; ptr < buffer + length; ) {
                inotify_event* event = reinterpret_cast<inotify_event*>(ptr);
                
                if (event->mask & IN_CREATE) {
                    std::cout << "File created: ";
                } else if (event->mask & IN_DELETE) {
                    std::cout << "File deleted: ";
                } else if (event->mask & IN_MODIFY) {
                    std::cout << "File modified: ";
                } else if (event->mask & IN_MOVE) {
                    std::cout << "File moved: ";
                }
                
                if (event->len > 0) {
                    std::cout << event->name << std::endl;
                }
                
                ptr += eventSize + event->len;
            }
        }
    }
};

int main() {
    FileWatcher watcher;
    
    watcher.addWatch(".", IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVE);
    
    std::cout << "Monitoring current directory. Press Ctrl+C to exit." << std::endl;
    
    watcher.processEvents();
    
    return 0;
}