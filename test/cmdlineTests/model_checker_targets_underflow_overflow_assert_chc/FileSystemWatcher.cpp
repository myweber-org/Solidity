
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileWatcher {
public:
    explicit FileWatcher(const fs::path& path) : watch_path(path) {
        if (!fs::exists(watch_path)) {
            throw std::runtime_error("Path does not exist");
        }
        update_file_states();
    }

    void start_watching(int interval_seconds = 1) {
        std::cout << "Watching: " << watch_path.string() << std::endl;
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

    void stop() {
        running = false;
    }

private:
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_states;
    bool running = true;

    void update_file_states() {
        file_states.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                file_states[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void check_for_changes() {
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                std::string file_path = entry.path().string();
                auto current_time = fs::last_write_time(entry);

                if (file_states.find(file_path) == file_states.end()) {
                    std::cout << "New file detected: " << file_path << std::endl;
                    file_states[file_path] = current_time;
                } else if (file_states[file_path] != current_time) {
                    std::cout << "File modified: " << file_path << std::endl;
                    file_states[file_path] = current_time;
                }
            }
        }

        std::vector<std::string> to_remove;
        for (const auto& [file_path, _] : file_states) {
            if (!fs::exists(file_path)) {
                std::cout << "File deleted: " << file_path << std::endl;
                to_remove.push_back(file_path);
            }
        }

        for (const auto& file_path : to_remove) {
            file_states.erase(file_path);
        }
    }
};

int main() {
    try {
        FileWatcher watcher(".");
        watcher.start_watching(2);
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
#include <functional>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher() : running_(false) {}

    void addWatchPath(const fs::path& path, Callback callback) {
        if (!fs::exists(path)) {
            std::cerr << "Path does not exist: " << path << std::endl;
            return;
        }

        watch_paths_[path] = {callback, getFileModificationMap(path)};
        std::cout << "Watching path: " << path << std::endl;
    }

    void start() {
        running_ = true;
        monitorThread_ = std::thread(&FileSystemWatcher::monitor, this);
    }

    void stop() {
        running_ = false;
        if (monitorThread_.joinable()) {
            monitorThread_.join();
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

private:
    struct WatchInfo {
        Callback callback;
        std::unordered_map<std::string, fs::file_time_type> file_mod_times;
    };

    std::unordered_map<fs::path, WatchInfo> watch_paths_;
    std::thread monitorThread_;
    bool running_;

    std::unordered_map<std::string, fs::file_time_type> getFileModificationMap(const fs::path& directory) {
        std::unordered_map<std::string, fs::file_time_type> mod_times;
        try {
            for (const auto& entry : fs::recursive_directory_iterator(directory)) {
                if (fs::is_regular_file(entry.path())) {
                    mod_times[entry.path().string()] = fs::last_write_time(entry.path());
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
        return mod_times;
    }

    void monitor() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            for (auto& [path, watch_info] : watch_paths_) {
                try {
                    auto current_mod_times = getFileModificationMap(path);
                    auto& old_mod_times = watch_info.file_mod_times;

                    for (const auto& [file_path, mod_time] : current_mod_times) {
                        auto it = old_mod_times.find(file_path);
                        if (it == old_mod_times.end()) {
                            std::cout << "File created: " << file_path << std::endl;
                            watch_info.callback(file_path, "created");
                        } else if (it->second != mod_time) {
                            std::cout << "File modified: " << file_path << std::endl;
                            watch_info.callback(file_path, "modified");
                            it->second = mod_time;
                        }
                    }

                    for (auto it = old_mod_times.begin(); it != old_mod_times.end();) {
                        if (current_mod_times.find(it->first) == current_mod_times.end()) {
                            std::cout << "File deleted: " << it->first << std::endl;
                            watch_info.callback(it->first, "deleted");
                            it = old_mod_times.erase(it);
                        } else {
                            ++it;
                        }
                    }

                    old_mod_times = std::move(current_mod_times);
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Error watching path " << path << ": " << e.what() << std::endl;
                }
            }
        }
    }
};

void exampleCallback(const fs::path& file_path, const std::string& action) {
    std::cout << "Callback: File " << file_path << " was " << action << std::endl;
}

int main() {
    FileSystemWatcher watcher;

    watcher.addWatchPath("./test_directory", exampleCallback);
    watcher.addWatchPath("./another_directory", [](const fs::path& p, const std::string& a) {
        std::cout << "Lambda callback: " << p << " - " << a << std::endl;
    });

    watcher.start();

    std::cout << "File system watcher started. Press Enter to stop..." << std::endl;
    std::cin.get();

    watcher.stop();
    return 0;
}