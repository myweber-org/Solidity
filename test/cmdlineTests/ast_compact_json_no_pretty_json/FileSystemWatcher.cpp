
#include <sys/inotify.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <limits>

class FileSystemWatcher {
private:
    int inotifyFd;
    std::vector<int> watchDescriptors;

public:
    FileSystemWatcher() : inotifyFd(-1) {
        inotifyFd = inotify_init();
        if (inotifyFd < 0) {
            std::cerr << "Failed to initialize inotify" << std::endl;
        }
    }

    ~FileSystemWatcher() {
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
        std::cout << "Watching: " << path << " (WD: " << wd << ")" << std::endl;
        return true;
    }

    void processEvents() {
        const size_t eventSize = sizeof(inotify_event);
        const size_t bufferSize = 1024 * (eventSize + 16);
        char buffer[bufferSize];

        while (true) {
            ssize_t length = read(inotifyFd, buffer, bufferSize);
            if (length < 0) {
                std::cerr << "Error reading inotify events" << std::endl;
                break;
            }

            size_t i = 0;
            while (i < static_cast<size_t>(length)) {
                inotify_event* event = reinterpret_cast<inotify_event*>(&buffer[i]);
                if (event->len) {
                    handleEvent(event);
                }
                i += eventSize + event->len;
            }
        }
    }

private:
    void handleEvent(const inotify_event* event) {
        std::string eventTypes;
        if (event->mask & IN_ACCESS)        eventTypes += "ACCESS ";
        if (event->mask & IN_MODIFY)        eventTypes += "MODIFY ";
        if (event->mask & IN_ATTRIB)        eventTypes += "ATTRIB ";
        if (event->mask & IN_CLOSE_WRITE)   eventTypes += "CLOSE_WRITE ";
        if (event->mask & IN_CLOSE_NOWRITE) eventTypes += "CLOSE_NOWRITE ";
        if (event->mask & IN_OPEN)          eventTypes += "OPEN ";
        if (event->mask & IN_MOVED_FROM)    eventTypes += "MOVED_FROM ";
        if (event->mask & IN_MOVED_TO)      eventTypes += "MOVED_TO ";
        if (event->mask & IN_CREATE)        eventTypes += "CREATE ";
        if (event->mask & IN_DELETE)        eventTypes += "DELETE ";
        if (event->mask & IN_DELETE_SELF)   eventTypes += "DELETE_SELF ";
        if (event->mask & IN_MOVE_SELF)     eventTypes += "MOVE_SELF ";

        std::cout << "WD:" << event->wd << " ";
        std::cout << "Mask:" << event->mask << " ";
        std::cout << "Events:[" << eventTypes << "] ";
        if (event->len > 0) {
            std::cout << "Name:" << event->name;
        }
        std::cout << std::endl;
    }
};

int main() {
    FileSystemWatcher watcher;

    watcher.addWatch(".", IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVE);
    std::cout << "Monitoring current directory. Press Ctrl+C to stop." << std::endl;

    watcher.processEvents();

    return 0;
}