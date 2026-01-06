
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    SimpleFileWatcher(const fs::path& path) : watch_path(path) {
        if (!fs::exists(path)) {
            throw std::runtime_error("Path does not exist: " + path.string());
        }
        last_check_time = fs::last_write_time(path);
        snapshot = collectFileSnapshot();
    }

    void startWatching(int interval_seconds = 1) {
        std::cout << "Watching: " << watch_path << std::endl;
        while (!should_stop) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            checkForChanges();
        }
    }

    void stopWatching() {
        should_stop = true;
    }

private:
    using FileSnapshot = std::unordered_map<std::string, fs::file_time_type>;

    FileSnapshot collectFileSnapshot() {
        FileSnapshot snap;
        if (fs::is_directory(watch_path)) {
            for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
                if (fs::is_regular_file(entry.path())) {
                    snap[entry.path().string()] = fs::last_write_time(entry.path());
                }
            }
        } else {
            snap[watch_path.string()] = fs::last_write_time(watch_path);
        }
        return snap;
    }

    void checkForChanges() {
        auto current_snapshot = collectFileSnapshot();
        auto current_time = fs::last_write_time(watch_path);

        if (current_time != last_check_time) {
            std::cout << "[Change] Root path modified: " << watch_path << std::endl;
            last_check_time = current_time;
        }

        for (const auto& [path, mtime] : current_snapshot) {
            auto it = snapshot.find(path);
            if (it == snapshot.end()) {
                std::cout << "[New] " << path << std::endl;
            } else if (it->second != mtime) {
                std::cout << "[Modified] " << path << std::endl;
            }
        }

        for (const auto& [path, mtime] : snapshot) {
            if (current_snapshot.find(path) == current_snapshot.end()) {
                std::cout << "[Deleted] " << path << std::endl;
            }
        }

        snapshot = std::move(current_snapshot);
    }

    fs::path watch_path;
    fs::file_time_type last_check_time;
    FileSnapshot snapshot;
    bool should_stop = false;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_watch>" << std::endl;
        return 1;
    }

    try {
        SimpleFileWatcher watcher(argv[1]);
        watcher.startWatching();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileWatcher {
public:
    FileWatcher(const std::string& path_to_watch, std::chrono::duration<int, std::milli> delay)
        : path_to_watch_{path_to_watch}, delay_{delay} {
        if (!fs::exists(path_to_watch_) || !fs::is_directory(path_to_watch_)) {
            throw std::invalid_argument("Path does not exist or is not a directory");
        }
        for (const auto& entry : fs::recursive_directory_iterator(path_to_watch_)) {
            if (fs::is_regular_file(entry.path())) {
                paths_[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void start(const std::function<void (const std::string&, FileStatus)>& action) {
        while (running_) {
            std::this_thread::sleep_for(delay_);
            auto it = paths_.begin();
            while (it != paths_.end()) {
                if (!fs::exists(it->first)) {
                    action(it->first, FileStatus::erased);
                    it = paths_.erase(it);
                } else {
                    ++it;
                }
            }

            for (const auto& entry : fs::recursive_directory_iterator(path_to_watch_)) {
                if (fs::is_regular_file(entry.path())) {
                    auto current_file_last_write_time = fs::last_write_time(entry.path());
                    std::string file_path = entry.path().string();
                    if (!contains(file_path)) {
                        paths_[file_path] = current_file_last_write_time;
                        action(file_path, FileStatus::created);
                    } else {
                        if (paths_[file_path] != current_file_last_write_time) {
                            paths_[file_path] = current_file_last_write_time;
                            action(file_path, FileStatus::modified);
                        }
                    }
                }
            }
        }
    }

    void stop() {
        running_ = false;
    }

    enum class FileStatus { created, modified, erased };

private:
    std::unordered_map<std::string, fs::file_time_type> paths_;
    std::string path_to_watch_;
    std::chrono::duration<int, std::milli> delay_;
    bool running_ = true;

    bool contains(const std::string& key) {
        return paths_.find(key) != paths_.end();
    }
};

int main() {
    std::string path_to_watch = ".";
    FileWatcher fw(path_to_watch, std::chrono::milliseconds(1000));

    auto handle_file_event = [](const std::string& path, FileWatcher::FileStatus status) {
        switch (status) {
            case FileWatcher::FileStatus::created:
                std::cout << "File created: " << path << std::endl;
                break;
            case FileWatcher::FileStatus::modified:
                std::cout << "File modified: " << path << std::endl;
                break;
            case FileWatcher::FileStatus::erased:
                std::cout << "File erased: " << path << std::endl;
                break;
        }
    };

    std::cout << "Watching directory: " << path_to_watch << std::endl;
    std::cout << "Press Ctrl+C to stop..." << std::endl;

    fw.start(handle_file_event);

    return 0;
}