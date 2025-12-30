
#include <sys/inotify.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <iostream>

class FileSystemWatcher {
private:
    int inotify_fd;
    std::map<int, std::string> watch_descriptors;

    bool add_watch(const std::string& path, uint32_t mask) {
        int wd = inotify_add_watch(inotify_fd, path.c_str(), mask);
        if (wd == -1) {
            perror("inotify_add_watch");
            return false;
        }
        watch_descriptors[wd] = path;
        std::cout << "Watching: " << path << " (wd=" << wd << ")\n";
        return true;
    }

public:
    FileSystemWatcher() : inotify_fd(-1) {}

    ~FileSystemWatcher() {
        if (inotify_fd != -1) {
            for (const auto& entry : watch_descriptors) {
                inotify_rm_watch(inotify_fd, entry.first);
            }
            close(inotify_fd);
        }
    }

    bool initialize() {
        inotify_fd = inotify_init();
        if (inotify_fd == -1) {
            perror("inotify_init");
            return false;
        }
        return true;
    }

    bool watch_directory(const std::string& path) {
        uint32_t mask = IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO;
        return add_watch(path, mask);
    }

    bool watch_file(const std::string& path) {
        uint32_t mask = IN_MODIFY | IN_DELETE_SELF | IN_MOVE_SELF;
        return add_watch(path, mask);
    }

    void process_events() {
        const size_t event_size = sizeof(struct inotify_event);
        const size_t buffer_size = 1024 * (event_size + 16);
        char buffer[buffer_size];

        while (true) {
            ssize_t length = read(inotify_fd, buffer, buffer_size);
            if (length == -1) {
                perror("read");
                break;
            }

            size_t i = 0;
            while (i < static_cast<size_t>(length)) {
                struct inotify_event* event = reinterpret_cast<struct inotify_event*>(&buffer[i]);
                if (event->len) {
                    std::string path = watch_descriptors[event->wd];
                    std::string event_name = (event->name) ? std::string(event->name) : "";
                    
                    std::cout << "Event detected on " << path;
                    if (!event_name.empty()) {
                        std::cout << "/" << event_name;
                    }
                    std::cout << " [";
                    
                    if (event->mask & IN_CREATE) std::cout << "CREATED ";
                    if (event->mask & IN_DELETE) std::cout << "DELETED ";
                    if (event->mask & IN_MODIFY) std::cout << "MODIFIED ";
                    if (event->mask & IN_MOVED_FROM) std::cout << "MOVED_FROM ";
                    if (event->mask & IN_MOVED_TO) std::cout << "MOVED_TO ";
                    if (event->mask & IN_DELETE_SELF) std::cout << "DELETED_SELF ";
                    if (event->mask & IN_MOVE_SELF) std::cout << "MOVED_SELF ";
                    
                    std::cout << "]\n";
                }
                i += event_size + event->len;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path1> [path2 ...]\n";
        return 1;
    }

    FileSystemWatcher watcher;
    if (!watcher.initialize()) {
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        std::string path = argv[i];
        struct stat path_stat;
        if (stat(path.c_str(), &path_stat) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISDIR(path_stat.st_mode)) {
            watcher.watch_directory(path);
        } else {
            watcher.watch_file(path);
        }
    }

    std::cout << "Monitoring started. Press Ctrl+C to exit.\n";
    watcher.process_events();

    return 0;
}