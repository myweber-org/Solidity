#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <mutex>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher() : running_(false) {}

    void addWatchPath(const fs::path& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (fs::exists(path) && fs::is_directory(path)) {
            watch_paths_.push_back(fs::canonical(path));
            std::cout << "Watching directory: " << watch_paths_.back() << std::endl;
        }
    }

    void setEventCallback(Callback callback) {
        callback_ = std::move(callback);
    }

    void start() {
        running_ = true;
        worker_thread_ = std::thread(&FileSystemWatcher::watchLoop, this);
    }

    void stop() {
        running_ = false;
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

private:
    struct FileState {
        std::uintmax_t size;
        std::time_t last_write_time;
    };

    void watchLoop() {
        std::unordered_map<fs::path, FileState> file_states;

        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto& watch_path : watch_paths_) {
                try {
                    for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
                        if (!fs::is_regular_file(entry.status())) {
                            continue;
                        }

                        const auto& path = entry.path();
                        auto current_state = FileState{
                            fs::file_size(path),
                            fs::last_write_time(path).time_since_epoch().count()
                        };

                        auto it = file_states.find(path);
                        if (it == file_states.end()) {
                            file_states[path] = current_state;
                            if (callback_) {
                                callback_(path, "CREATED");
                            }
                        } else {
                            if (it->second.size != current_state.size ||
                                it->second.last_write_time != current_state.last_write_time) {
                                it->second = current_state;
                                if (callback_) {
                                    callback_(path, "MODIFIED");
                                }
                            }
                        }
                    }

                    for (auto it = file_states.begin(); it != file_states.end(); ) {
                        if (!fs::exists(it->first)) {
                            if (callback_) {
                                callback_(it->first, "DELETED");
                            }
                            it = file_states.erase(it);
                        } else {
                            ++it;
                        }
                    }
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Filesystem error: " << e.what() << std::endl;
                }
            }
        }
    }

    std::vector<fs::path> watch_paths_;
    Callback callback_;
    std::atomic<bool> running_;
    std::thread worker_thread_;
    std::mutex mutex_;
};

int main() {
    FileSystemWatcher watcher;

    watcher.setEventCallback([](const fs::path& path, const std::string& event) {
        std::cout << "Event: " << event << " - " << path.filename().string() << std::endl;
    });

    watcher.addWatchPath(".");
    watcher.start();

    std::cout << "File system watcher started. Press Enter to stop..." << std::endl;
    std::cin.get();

    watcher.stop();
    return 0;
}