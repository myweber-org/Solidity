
#include <sys/inotify.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

class FileSystemWatcher {
private:
    int inotifyFd;
    std::map<int, std::string> watchDescriptors;
    
public:
    FileSystemWatcher() : inotifyFd(-1) {
        inotifyFd = inotify_init();
        if (inotifyFd < 0) {
            throw std::runtime_error("Failed to initialize inotify");
        }
    }
    
    ~FileSystemWatcher() {
        for (const auto& entry : watchDescriptors) {
            inotify_rm_watch(inotifyFd, entry.first);
        }
        if (inotifyFd >= 0) {
            close(inotifyFd);
        }
    }
    
    void addWatch(const std::string& path, uint32_t mask) {
        int wd = inotify_add_watch(inotifyFd, path.c_str(), mask);
        if (wd < 0) {
            throw std::runtime_error("Failed to add watch for: " + path);
        }
        watchDescriptors[wd] = path;
        std::cout << "Watching: " << path << " (wd=" << wd << ")" << std::endl;
    }
    
    void removeWatch(int wd) {
        if (watchDescriptors.find(wd) != watchDescriptors.end()) {
            inotify_rm_watch(inotifyFd, wd);
            watchDescriptors.erase(wd);
            std::cout << "Removed watch: wd=" << wd << std::endl;
        }
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
            struct inotify_event* event = reinterpret_cast<struct inotify_event*>(&buffer[i]);
            
            if (watchDescriptors.find(event->wd) != watchDescriptors.end()) {
                std::string path = watchDescriptors[event->wd];
                std::cout << "Event on: " << path;
                
                if (event->len > 0) {
                    std::cout << "/" << event->name;
                }
                
                std::cout << " - Events:";
                
                if (event->mask & IN_ACCESS) std::cout << " IN_ACCESS";
                if (event->mask & IN_MODIFY) std::cout << " IN_MODIFY";
                if (event->mask & IN_ATTRIB) std::cout << " IN_ATTRIB";
                if (event->mask & IN_CLOSE_WRITE) std::cout << " IN_CLOSE_WRITE";
                if (event->mask & IN_CLOSE_NOWRITE) std::cout << " IN_CLOSE_NOWRITE";
                if (event->mask & IN_OPEN) std::cout << " IN_OPEN";
                if (event->mask & IN_MOVED_FROM) std::cout << " IN_MOVED_FROM";
                if (event->mask & IN_MOVED_TO) std::cout << " IN_MOVED_TO";
                if (event->mask & IN_CREATE) std::cout << " IN_CREATE";
                if (event->mask & IN_DELETE) std::cout << " IN_DELETE";
                if (event->mask & IN_DELETE_SELF) std::cout << " IN_DELETE_SELF";
                if (event->mask & IN_MOVE_SELF) std::cout << " IN_MOVE_SELF";
                
                std::cout << std::endl;
            }
            
            i += eventSize + event->len;
        }
    }
    
    void listWatches() const {
        std::cout << "Active watches:" << std::endl;
        for (const auto& entry : watchDescriptors) {
            std::cout << "  wd=" << entry.first << " -> " << entry.second << std::endl;
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher;
        
        watcher.addWatch(".", IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO);
        watcher.addWatch("/tmp", IN_CREATE | IN_DELETE);
        
        watcher.listWatches();
        
        std::cout << "\nMonitoring file system events (press Ctrl+C to exit)..." << std::endl;
        
        while (true) {
            watcher.processEvents();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}