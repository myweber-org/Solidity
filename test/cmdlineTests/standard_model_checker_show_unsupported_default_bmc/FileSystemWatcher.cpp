
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>
#include <functional>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using FilePath = fs::path;
    using FileTime = fs::file_time_type;
    using Callback = std::function<void(const FilePath&, const std::string&)>;

    FileSystemWatcher() = default;

    void addWatchPath(const FilePath& path) {
        if (fs::exists(path)) {
            updateFileState(path);
            watchPaths.push_back(path);
        }
    }

    void setCallback(Callback cb) {
        callback = std::move(cb);
    }

    void startMonitoring(int intervalMs = 1000) {
        monitoring = true;
        while (monitoring) {
            checkForChanges();
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
        }
    }

    void stopMonitoring() {
        monitoring = false;
    }

private:
    struct FileState {
        FileTime lastWriteTime;
        uintmax_t fileSize;
        bool exists;
    };

    std::vector<FilePath> watchPaths;
    std::unordered_map<std::string, FileState> fileStates;
    Callback callback;
    bool monitoring = false;

    void updateFileState(const FilePath& path) {
        std::string key = path.string();
        FileState state;

        if (fs::exists(path)) {
            state.lastWriteTime = fs::last_write_time(path);
            state.fileSize = fs::file_size(path);
            state.exists = true;
        } else {
            state.exists = false;
        }

        fileStates[key] = state;
    }

    void checkForChanges() {
        for (const auto& watchPath : watchPaths) {
            if (fs::is_directory(watchPath)) {
                checkDirectory(watchPath);
            } else {
                checkFile(watchPath);
            }
        }
    }

    void checkDirectory(const FilePath& dirPath) {
        try {
            for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
                if (entry.is_regular_file()) {
                    checkFile(entry.path());
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
    }

    void checkFile(const FilePath& filePath) {
        std::string key = filePath.string();
        auto it = fileStates.find(key);

        bool currentExists = fs::exists(filePath);
        FileState newState;

        if (currentExists) {
            newState.lastWriteTime = fs::last_write_time(filePath);
            newState.fileSize = fs::file_size(filePath);
            newState.exists = true;
        } else {
            newState.exists = false;
        }

        if (it == fileStates.end()) {
            if (currentExists && callback) {
                callback(filePath, "CREATED");
            }
        } else {
            const FileState& oldState = it->second;

            if (!oldState.exists && currentExists) {
                if (callback) callback(filePath, "CREATED");
            } else if (oldState.exists && !currentExists) {
                if (callback) callback(filePath, "DELETED");
            } else if (oldState.exists && currentExists) {
                if (oldState.lastWriteTime != newState.lastWriteTime ||
                    oldState.fileSize != newState.fileSize) {
                    if (callback) callback(filePath, "MODIFIED");
                }
            }
        }

        fileStates[key] = newState;
    }
};

int main() {
    FileSystemWatcher watcher;

    watcher.setCallback([](const FileSystemWatcher::FilePath& path, const std::string& action) {
        std::cout << "File: " << path << " Action: " << action << std::endl;
    });

    watcher.addWatchPath(".");
    
    std::thread monitorThread([&watcher]() {
        watcher.startMonitoring(500);
    });

    std::cout << "Monitoring started. Press Enter to stop..." << std::endl;
    std::cin.get();

    watcher.stopMonitoring();
    if (monitorThread.joinable()) {
        monitorThread.join();
    }

    return 0;
}#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class DirectoryWatcher {
public:
    explicit DirectoryWatcher(const fs::path& dir_path) : watch_path(dir_path) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Invalid directory path provided");
        }
        refresh_file_list();
    }

    void start_monitoring(int interval_seconds = 2) {
        std::cout << "Monitoring directory: " << watch_path.string() << std::endl;
        std::cout << "Check interval: " << interval_seconds << " seconds" << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path watch_path;
    std::unordered_set<std::string> known_files;

    void refresh_file_list() {
        known_files.clear();
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                known_files.insert(entry.path().filename().string());
            }
        }
    }

    void check_for_changes() {
        std::unordered_set<std::string> current_files;

        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                current_files.insert(entry.path().filename().string());
            }
        }

        for (const auto& file : current_files) {
            if (known_files.find(file) == known_files.end()) {
                std::cout << "[+] New file detected: " << file << std::endl;
            }
        }

        for (const auto& file : known_files) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "[-] File removed: " << file << std::endl;
            }
        }

        known_files = std::move(current_files);
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        DirectoryWatcher watcher(argv[1]);
        watcher.start_monitoring();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}