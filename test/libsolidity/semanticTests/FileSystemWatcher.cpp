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
    FileSystemWatcher(const std::string& path) {
        inotifyFd = inotify_init();
        if (inotifyFd < 0) {
            std::cerr << "Error initializing inotify" << std::endl;
            return;
        }

        watchDescriptor = inotify_add_watch(inotifyFd, path.c_str(), 
                                           IN_MODIFY | IN_CREATE | IN_DELETE);
        if (watchDescriptor < 0) {
            std::cerr << "Error adding watch for: " << path << std::endl;
            close(inotifyFd);
            return;
        }

        std::cout << "Watching directory: " << path << std::endl;
    }

    void startMonitoring() {
        char buffer[BUF_LEN];
        while (true) {
            ssize_t length = read(inotifyFd, buffer, BUF_LEN);
            if (length < 0) {
                std::cerr << "Error reading inotify events" << std::endl;
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
                    if (event->mask & IN_MODIFY) {
                        std::cout << "File modified: " << event->name << std::endl;
                    }
                    if (event->mask & IN_DELETE) {
                        std::cout << "File deleted: " << event->name << std::endl;
                    }
                }
                i += EVENT_SIZE + event->len;
            }
        }
    }

    ~FileSystemWatcher() {
        if (inotifyFd >= 0) {
            inotify_rm_watch(inotifyFd, watchDescriptor);
            close(inotifyFd);
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    FileSystemWatcher watcher(argv[1]);
    watcher.startMonitoring();

    return 0;
}