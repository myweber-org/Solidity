
#include <sys/inotify.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <iostream>
#include <cstring>
#include <errno.h>

class FileWatcher {
public:
    using Callback = std::function<void(const std::string&, uint32_t)>;
    
    FileWatcher() : inotify_fd(-1) {
        inotify_fd = inotify_init();
        if (inotify_fd < 0) {
            throw std::runtime_error("Failed to initialize inotify");
        }
    }
    
    ~FileWatcher() {
        for (auto& wd : watch_descriptors) {
            inotify_rm_watch(inotify_fd, wd.first);
        }
        if (inotify_fd >= 0) {
            close(inotify_fd);
        }
    }
    
    void addWatch(const std::string& path, uint32_t mask, Callback callback) {
        int wd = inotify_add_watch(inotify_fd, path.c_str(), mask);
        if (wd < 0) {
            throw std::runtime_error("Failed to add watch for " + path + ": " + strerror(errno));
        }
        watch_descriptors[wd] = {path, callback};
    }
    
    void removeWatch(int wd) {
        auto it = watch_descriptors.find(wd);
        if (it != watch_descriptors.end()) {
            inotify_rm_watch(inotify_fd, wd);
            watch_descriptors.erase(it);
        }
    }
    
    void processEvents(int timeout_ms = 100) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(inotify_fd, &fds);
        
        timeval timeout;
        timeout.tv_sec = timeout_ms / 1000;
        timeout.tv_usec = (timeout_ms % 1000) * 1000;
        
        int ret = select(inotify_fd + 1, &fds, nullptr, nullptr, &timeout);
        if (ret < 0) {
            throw std::runtime_error("select failed: " + std::string(strerror(errno)));
        }
        
        if (ret == 0) {
            return;
        }
        
        char buffer[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
        ssize_t len = read(inotify_fd, buffer, sizeof(buffer));
        
        if (len < 0) {
            throw std::runtime_error("read failed: " + std::string(strerror(errno)));
        }
        
        char* ptr = buffer;
        while (ptr < buffer + len) {
            struct inotify_event* event = reinterpret_cast<struct inotify_event*>(ptr);
            auto it = watch_descriptors.find(event->wd);
            if (it != watch_descriptors.end()) {
                std::string full_path = it->second.first;
                if (event->len > 0) {
                    full_path += "/" + std::string(event->name);
                }
                it->second.second(full_path, event->mask);
            }
            ptr += sizeof(struct inotify_event) + event->len;
        }
    }
    
private:
    int inotify_fd;
    std::map<int, std::pair<std::string, Callback>> watch_descriptors;
};