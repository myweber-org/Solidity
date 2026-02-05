#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path path_to_watch;
    std::unordered_set<std::string> current_files;

    void populateFileSet() {
        current_files.clear();
        if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                current_files.insert(entry.path().filename().string());
            }
        }
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        populateFileSet();
    }

    void startMonitoring(int interval_seconds = 2) {
        std::cout << "Starting to monitor: " << path_to_watch << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            checkForChanges();
        }
    }

    void checkForChanges() {
        auto old_files = current_files;
        populateFileSet();

        // Check for new files
        for (const auto& file : current_files) {
            if (old_files.find(file) == old_files.end()) {
                std::cout << "[+] New file detected: " << file << std::endl;
            }
        }

        // Check for deleted files
        for (const auto& file : old_files) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "[-] File deleted: " << file << std::endl;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    std::string watch_path = argv[1];
    FileSystemWatcher watcher(watch_path);
    watcher.startMonitoring();

    return 0;
}
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>
#include <functional>

namespace fs = std::filesystem;

class SimpleFileWatcher {
public:
    using FileChangeCallback = std::function<void(const fs::path&, const std::string&)>;

    SimpleFileWatcher(const fs::path& watch_path, FileChangeCallback callback)
        : watch_directory(watch_path), change_callback(callback), running(false) {
        if (!fs::exists(watch_directory) || !fs::is_directory(watch_directory)) {
            throw std::runtime_error("Invalid watch directory path");
        }
        initialize_file_states();
    }

    ~SimpleFileWatcher() {
        stop();
    }

    void start() {
        running = true;
        watcher_thread = std::thread(&SimpleFileWatcher::watch_loop, this);
    }

    void stop() {
        running = false;
        if (watcher_thread.joinable()) {
            watcher_thread.join();
        }
    }

private:
    struct FileState {
        std::time_t last_write_time;
        std::uintmax_t file_size;
    };

    void initialize_file_states() {
        for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
            if (fs::is_regular_file(entry.path())) {
                auto ftime = fs::last_write_time(entry.path());
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
                std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

                file_states[entry.path()] = FileState{
                    cftime,
                    fs::file_size(entry.path())
                };
            }
        }
    }

    void watch_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            std::unordered_map<fs::path, FileState> current_states;

            for (const auto& entry : fs::recursive_directory_iterator(watch_directory)) {
                if (fs::is_regular_file(entry.path())) {
                    auto ftime = fs::last_write_time(entry.path());
                    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
                    std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

                    current_states[entry.path()] = FileState{
                        cftime,
                        fs::file_size(entry.path())
                    };

                    auto it = file_states.find(entry.path());
                    if (it == file_states.end()) {
                        change_callback(entry.path(), "CREATED");
                    } else if (it->second.last_write_time != cftime || it->second.file_size != current_states[entry.path()].file_size) {
                        change_callback(entry.path(), "MODIFIED");
                    }
                }
            }

            for (const auto& [path, state] : file_states) {
                if (current_states.find(path) == current_states.end()) {
                    change_callback(path, "DELETED");
                }
            }

            file_states = std::move(current_states);
        }
    }

    fs::path watch_directory;
    FileChangeCallback change_callback;
    std::unordered_map<fs::path, FileState> file_states;
    std::thread watcher_thread;
    std::atomic<bool> running;
};

void example_callback(const fs::path& file_path, const std::string& change_type) {
    std::cout << "File: " << file_path << " - Change: " << change_type << std::endl;
}

int main() {
    try {
        SimpleFileWatcher watcher(fs::current_path(), example_callback);
        watcher.start();

        std::cout << "Watching directory: " << fs::current_path() << std::endl;
        std::cout << "Press Enter to stop watching..." << std::endl;
        std::cin.get();

        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}