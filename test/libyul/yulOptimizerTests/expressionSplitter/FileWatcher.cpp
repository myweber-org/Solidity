
#include <sys/inotify.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>

class FileWatcher {
private:
    int inotifyFd;
    std::map<int, std::string> watchDescriptors;

public:
    FileWatcher() : inotifyFd(-1) {
        inotifyFd = inotify_init();
        if (inotifyFd < 0) {
            perror("inotify_init");
            exit(EXIT_FAILURE);
        }
    }

    ~FileWatcher() {
        for (auto& pair : watchDescriptors) {
            inotify_rm_watch(inotifyFd, pair.first);
        }
        close(inotifyFd);
    }

    bool addWatch(const std::string& path) {
        int wd = inotify_add_watch(inotifyFd, path.c_str(), IN_MODIFY | IN_CREATE | IN_DELETE);
        if (wd < 0) {
            perror("inotify_add_watch");
            return false;
        }
        watchDescriptors[wd] = path;
        printf("Watching: %s\n", path.c_str());
        return true;
    }

    void processEvents() {
        const size_t eventSize = sizeof(struct inotify_event);
        const size_t bufferSize = 1024 * (eventSize + 16);
        char buffer[bufferSize];

        while (true) {
            ssize_t length = read(inotifyFd, buffer, bufferSize);
            if (length < 0) {
                perror("read");
                break;
            }

            size_t i = 0;
            while (i < length) {
                struct inotify_event* event = reinterpret_cast<struct inotify_event*>(&buffer[i]);
                if (event->len) {
                    std::string path = watchDescriptors[event->wd];
                    if (event->mask & IN_CREATE) {
                        printf("File created: %s/%s\n", path.c_str(), event->name);
                    }
                    if (event->mask & IN_MODIFY) {
                        printf("File modified: %s/%s\n", path.c_str(), event->name);
                    }
                    if (event->mask & IN_DELETE) {
                        printf("File deleted: %s/%s\n", path.c_str(), event->name);
                    }
                }
                i += eventSize + event->len;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <directory1> [directory2] ...\n", argv[0]);
        return EXIT_FAILURE;
    }

    FileWatcher watcher;

    for (int i = 1; i < argc; ++i) {
        if (!watcher.addWatch(argv[i])) {
            fprintf(stderr, "Failed to watch directory: %s\n", argv[i]);
        }
    }

    printf("Starting file system monitoring. Press Ctrl+C to exit.\n");
    watcher.processEvents();

    return EXIT_SUCCESS;
}