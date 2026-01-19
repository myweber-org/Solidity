
#include <sys/inotify.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>
#include <cstring>
#include <stdexcept>

class FileWatcher {
public:
    FileWatcher() : inotify_fd(inotify_init()) {
        if (inotify_fd < 0) {
            throw std::runtime_error("Failed to initialize inotify");
        }
    }

    ~FileWatcher() {
        if (inotify_fd >= 0) {
            close(inotify_fd);
        }
        for (int wd : watch_descriptors) {
            inotify_rm_watch(inotify_fd, wd);
        }
    }

    void add_watch(const std::string& path, uint32_t mask = IN_MODIFY | IN_CREATE | IN_DELETE) {
        int wd = inotify_add_watch(inotify_fd, path.c_str(), mask);
        if (wd < 0) {
            throw std::runtime_error("Failed to add watch for " + path);
        }
        watch_descriptors.push_back(wd);
        watched_paths[wd] = path;
    }

    void remove_watch(int wd) {
        if (inotify_rm_watch(inotify_fd, wd) < 0) {
            throw std::runtime_error("Failed to remove watch");
        }
        auto it = std::find(watch_descriptors.begin(), watch_descriptors.end(), wd);
        if (it != watch_descriptors.end()) {
            watch_descriptors.erase(it);
        }
        watched_paths.erase(wd);
    }

    struct Event {
        int wd;
        uint32_t mask;
        std::string path;
    };

    Event read_event() {
        char buffer[4096];
        ssize_t length = read(inotify_fd, buffer, sizeof(buffer));
        if (length < 0) {
            throw std::runtime_error("Failed to read inotify event");
        }

        Event event;
        int i = 0;
        while (i < length) {
            struct inotify_event* ievent = reinterpret_cast<struct inotify_event*>(&buffer[i]);
            event.wd = ievent->wd;
            event.mask = ievent->mask;
            if (ievent->len > 0) {
                event.path = watched_paths[ievent->wd] + "/" + std::string(ievent->name);
            } else {
                event.path = watched_paths[ievent->wd];
            }
            i += sizeof(struct inotify_event) + ievent->len;
        }
        return event;
    }

private:
    int inotify_fd;
    std::vector<int> watch_descriptors;
    std::unordered_map<int, std::string> watched_paths;
};