#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const fs::path& path) : watchPath(path) {
        if (fs::exists(path) && fs::is_directory(path)) {
            refreshFileMap();
        }
    }

    void startWatching(int intervalSeconds = 1) {
        std::cout << "Watching directory: " << watchPath.string() << std::endl;
        
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
            
            auto currentState = getCurrentFileState();
            
            for (const auto& [path, time] : currentState) {
                if (fileMap.find(path) == fileMap.end()) {
                    std::cout << "[CREATED] " << path << std::endl;
                }
            }
            
            for (const auto& [path, oldTime] : fileMap) {
                if (currentState.find(path) == currentState.end()) {
                    std::cout << "[DELETED] " << path << std::endl;
                } else {
                    auto newTime = currentState[path];
                    if (newTime != oldTime) {
                        std::cout << "[MODIFIED] " << path << std::endl;
                    }
                }
            }
            
            fileMap = std::move(currentState);
        }
    }

private:
    fs::path watchPath;
    std::unordered_map<std::string, fs::file_time_type> fileMap;

    void refreshFileMap() {
        fileMap.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watchPath)) {
            if (fs::is_regular_file(entry.path())) {
                fileMap[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    std::unordered_map<std::string, fs::file_time_type> getCurrentFileState() {
        std::unordered_map<std::string, fs::file_time_type> current;
        
        if (!fs::exists(watchPath)) {
            std::cout << "Warning: Directory no longer exists!" << std::endl;
            return current;
        }
        
        for (const auto& entry : fs::recursive_directory_iterator(watchPath)) {
            if (fs::is_regular_file(entry.path())) {
                current[entry.path().string()] = fs::last_write_time(entry);
            }
        }
        
        return current;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }
    
    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.startWatching();
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& directory) : watch_directory(directory) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        update_snapshot();
    }

    void start_watching(int interval_seconds = 2) {
        std::cout << "Watching directory: " << watch_directory << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path watch_directory;
    std::unordered_set<std::string> file_snapshot;

    void update_snapshot() {
        file_snapshot.clear();
        for (const auto& entry : fs::directory_iterator(watch_directory)) {
            file_snapshot.insert(entry.path().filename().string());
        }
    }

    void check_for_changes() {
        auto current_files = std::unordered_set<std::string>{};
        for (const auto& entry : fs::directory_iterator(watch_directory)) {
            current_files.insert(entry.path().filename().string());
        }

        for (const auto& file : current_files) {
            if (file_snapshot.find(file) == file_snapshot.end()) {
                std::cout << "New file detected: " << file << std::endl;
            }
        }

        for (const auto& file : file_snapshot) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "File removed: " << file << std::endl;
            }
        }

        file_snapshot = std::move(current_files);
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start_watching();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}