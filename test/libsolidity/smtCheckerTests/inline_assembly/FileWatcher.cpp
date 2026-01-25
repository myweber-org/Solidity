
#include <sys/inotify.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <map>

class FileWatcher {
private:
    int inotifyFd;
    std::map<int, std::string> watchDescriptors;
    
public:
    FileWatcher() : inotifyFd(-1) {
        inotifyFd = inotify_init();
        if (inotifyFd < 0) {
            std::cerr << "Failed to initialize inotify" << std::endl;
        }
    }
    
    ~FileWatcher() {
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
            std::cerr << "Failed to add watch for: " << path << std::endl;
            return false;
        }
        watchDescriptors[wd] = path;
        std::cout << "Watching: " << path << " (WD: " << wd << ")" << std::endl;
        return true;
    }
    
    void processEvents() {
        const size_t eventSize = sizeof(struct inotify_event);
        const size_t bufferSize = 1024 * (eventSize + 16);
        
        char buffer[bufferSize];
        ssize_t length = read(inotifyFd, buffer, bufferSize);
        
        if (length < 0) {
            std::cerr << "Error reading inotify events" << std::endl;
            return;
        }
        
        size_t i = 0;
        while (i < length) {
            struct inotify_event* event = (struct inotify_event*)&buffer[i];
            
            if (event->len) {
                std::string fileName = (event->name) ? std::string(event->name) : "";
                std::string path = watchDescriptors[event->wd];
                
                if (!fileName.empty()) {
                    path += "/" + fileName;
                }
                
                std::cout << "Event detected on: " << path << std::endl;
                
                if (event->mask & IN_CREATE) {
                    std::cout << "  File created" << std::endl;
                }
                if (event->mask & IN_DELETE) {
                    std::cout << "  File deleted" << std::endl;
                }
                if (event->mask & IN_MODIFY) {
                    std::cout << "  File modified" << std::endl;
                }
                if (event->mask & IN_MOVE) {
                    std::cout << "  File moved/renamed" << std::endl;
                }
            }
            
            i += eventSize + event->len;
        }
    }
    
    void removeWatch(int wd) {
        if (inotify_rm_watch(inotifyFd, wd) < 0) {
            std::cerr << "Failed to remove watch descriptor: " << wd << std::endl;
        } else {
            watchDescriptors.erase(wd);
            std::cout << "Removed watch: " << wd << std::endl;
        }
    }
};

int main() {
    FileWatcher watcher;
    
    watcher.addWatch(".", IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVE);
    
    std::cout << "Monitoring current directory. Press Ctrl+C to exit." << std::endl;
    
    while (true) {
        watcher.processEvents();
        sleep(1);
    }
    
    return 0;
}