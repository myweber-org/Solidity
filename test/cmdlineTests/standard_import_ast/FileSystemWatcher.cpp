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
            std::cerr << "Error initializing inotify" << std::endl;
        }
    }

    bool addWatch(const std::string& path) {
        if (inotifyFd < 0) return false;
        
        watchDescriptor = inotify_add_watch(inotifyFd, path.c_str(), 
                                           IN_MODIFY | IN_CREATE | IN_DELETE);
        if (watchDescriptor < 0) {
            std::cerr << "Error adding watch for: " << path << std::endl;
            return false;
        }
        return true;
    }

    void startMonitoring() {
        if (inotifyFd < 0 || watchDescriptor < 0) {
            std::cerr << "Watcher not properly initialized" << std::endl;
            return;
        }

        char buffer[BUF_LEN];
        std::cout << "Monitoring file system changes..." << std::endl;
        
        while (true) {
            ssize_t length = read(inotifyFd, buffer, BUF_LEN);
            if (length < 0) {
                std::cerr << "Error reading inotify events" << std::endl;
                break;
            }

            for (char* ptr = buffer; ptr < buffer + length; ) {
                struct inotify_event* event = reinterpret_cast<struct inotify_event*>(ptr);
                
                if (event->mask & IN_CREATE) {
                    std::cout << "File created: " << event->name << std::endl;
                }
                if (event->mask & IN_MODIFY) {
                    std::cout << "File modified: " << event->name << std::endl;
                }
                if (event->mask & IN_DELETE) {
                    std::cout << "File deleted: " << event->name << std::endl;
                }
                
                ptr += EVENT_SIZE + event->len;
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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    FileSystemWatcher watcher;
    if (watcher.addWatch(argv[1])) {
        watcher.startMonitoring();
    }

    return 0;
}