
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <vector>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher() : running_(false) {}

    void addWatchPath(const fs::path& path, Callback callback) {
        if (!fs::exists(path)) {
            throw std::runtime_error("Path does not exist: " + path.string());
        }
        watch_paths_[path] = callback;
        file_states_[path] = getFileState(path);
    }

    void start() {
        running_ = true;
        monitor_thread_ = std::thread(&FileSystemWatcher::monitorLoop, this);
    }

    void stop() {
        running_ = false;
        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
    }

    ~FileSystemWatcher() {
        stop();
    }

private:
    struct FileState {
        std::uintmax_t size;
        std::time_t last_write_time;
        bool is_directory;

        bool operator==(const FileState& other) const {
            return size == other.size && last_write_time == other.last_write_time && is_directory == other.is_directory;
        }
    };

    std::unordered_map<fs::path, Callback> watch_paths_;
    std::unordered_map<fs::path, FileState> file_states_;
    std::atomic<bool> running_;
    std::thread monitor_thread_;

    FileState getFileState(const fs::path& path) {
        auto status = fs::status(path);
        auto last_write = fs::last_write_time(path);
        auto last_write_time_t = std::chrono::system_clock::to_time_t(
            std::chrono::file_clock::to_sys(last_write)
        );

        return {
            fs::is_directory(status) ? 0 : fs::file_size(path),
            last_write_time_t,
            fs::is_directory(status)
        };
    }

    void monitorLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            for (const auto& [path, callback] : watch_paths_) {
                if (!fs::exists(path)) {
                    callback(path, "deleted");
                    file_states_.erase(path);
                    continue;
                }

                FileState current_state = getFileState(path);
                FileState previous_state = file_states_[path];

                if (!(current_state == previous_state)) {
                    if (!previous_state.is_directory && !current_state.is_directory) {
                        if (current_state.size != previous_state.size) {
                            callback(path, "modified");
                        } else if (current_state.last_write_time != previous_state.last_write_time) {
                            callback(path, "updated");
                        }
                    } else if (!previous_state.is_directory && current_state.is_directory) {
                        callback(path, "became_directory");
                    } else if (previous_state.is_directory && !current_state.is_directory) {
                        callback(path, "became_file");
                    }
                    file_states_[path] = current_state;
                }

                if (current_state.is_directory) {
                    checkDirectoryContents(path, callback);
                }
            }
        }
    }

    void checkDirectoryContents(const fs::path& dir_path, Callback callback) {
        std::vector<fs::path> current_files;
        for (const auto& entry : fs::directory_iterator(dir_path)) {
            current_files.push_back(entry.path());
        }

        static std::unordered_map<fs::path, std::vector<fs::path>> previous_files;
        auto& previous = previous_files[dir_path];

        for (const auto& file : current_files) {
            if (std::find(previous.begin(), previous.end(), file) == previous.end()) {
                callback(file, "created");
            }
        }

        for (const auto& file : previous) {
            if (std::find(current_files.begin(), current_files.end(), file) == current_files.end()) {
                callback(file, "deleted");
            }
        }

        previous = std::move(current_files);
    }
};

int main() {
    try {
        FileSystemWatcher watcher;

        watcher.addWatchPath("./test_dir", [](const fs::path& path, const std::string& action) {
            std::cout << "File " << path << " action: " << action << std::endl;
        });

        watcher.addWatchPath("./test_file.txt", [](const fs::path& path, const std::string& action) {
            std::cout << "File " << path << " action: " << action << std::endl;
        });

        std::cout << "Starting file system watcher. Press Enter to stop." << std::endl;
        watcher.start();

        std::cin.get();
        watcher.stop();
        std::cout << "File system watcher stopped." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}